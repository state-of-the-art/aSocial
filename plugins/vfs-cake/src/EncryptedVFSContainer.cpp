// Copyright (C) 2026  aSocial Developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Author: Rabit (@rabits)

#include "EncryptedVFSContainer.h"
#include "VirtualFile.h"

#include <QFileInfo>
#include <QStorageInfo>
#include <QtEndian>
#include <QLoggingCategory>

#include <sodium.h>

Q_LOGGING_CATEGORY(EVFS, "EncryptedVFSContainer")

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

EncryptedVFSContainer::EncryptedVFSContainer(QObject* parent)
    : QObject(parent)
{
}

EncryptedVFSContainer::~EncryptedVFSContainer()
{
    if (m_isOpen)
        close();
}

// ---------------------------------------------------------------------------
// Open / close
// ---------------------------------------------------------------------------

bool EncryptedVFSContainer::open(const QString& containerPath,
                                  const QString& passphrase,
                                  quint64 maxContainerSize)
{
    if (m_isOpen)
        close();

    Q_ASSERT(crypto_aead_xchacha20poly1305_ietf_KEYBYTES  == KEY_SIZE);
    Q_ASSERT(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES == NONCE_SIZE);
    Q_ASSERT(crypto_aead_xchacha20poly1305_ietf_ABYTES    == MAC_SIZE);
    Q_ASSERT(crypto_pwhash_SALTBYTES == SALT_SIZE);

    const bool isNew = !QFile::exists(containerPath);
    if (isNew && !createContainer(containerPath, maxContainerSize))
        return false;

    m_file.setFileName(containerPath);
    if (!m_file.open(QIODevice::ReadWrite)) {
        qCCritical(EVFS) << "Cannot open container:" << containerPath;
        return false;
    }

    // Verify magic header
    QByteArray magic = m_file.read(MAGIC_SIZE);
    if (magic != QByteArray(MAGIC, MAGIC_SIZE)) {
        qCCritical(EVFS) << "Invalid magic header";
        m_file.close();
        return false;
    }

    // Read global salt
    m_salt = m_file.read(static_cast<qint64>(SALT_SIZE));
    if (static_cast<quint64>(m_salt.size()) != SALT_SIZE) {
        qCCritical(EVFS) << "Cannot read salt";
        m_file.close();
        return false;
    }

    // Derive the encryption key from passphrase + salt
    if (!deriveKey(passphrase, m_salt)) {
        qCCritical(EVFS) << "Key derivation failed (out of memory?)";
        m_file.close();
        return false;
    }

    m_isOpen = true;

    // Trial-decrypt every slot to discover files belonging to this key
    loadSlots();

    qCDebug(EVFS) << "Container opened:" << containerPath
                   << " files:" << m_files.size();
    return true;
}

void EncryptedVFSContainer::close()
{
    if (!m_isOpen)
        return;

    // Flush every still-open virtual file (copy the set because
    // VirtualFile::flush may call unregisterOpenFile)
    const QSet<VirtualFile*> snapshot = m_openFiles;
    for (VirtualFile* vf : snapshot)
        vf->flush();
    m_openFiles.clear();

    m_isOpen = false;
    m_file.close();

    // Secure-wipe sensitive material
    secureWipe(m_key);
    secureWipe(m_salt);
    m_files.clear();
}

// ---------------------------------------------------------------------------
// File operations (public API)
// ---------------------------------------------------------------------------

QStringList EncryptedVFSContainer::listFiles() const
{
    QStringList names;
    names.reserve(m_files.size());
    for (const auto& fp : m_files)
        names.append(fp.filename);
    return names;
}

QIODevice* EncryptedVFSContainer::openFile(const QString& filename,
                                            QIODevice::OpenMode mode)
{
    if (!m_isOpen)
        return nullptr;

    const int idx = findFileSlot(filename);
    QByteArray data;
    int slotIndex;

    if (idx >= 0) {
        // ---- Existing file ----
        slotIndex = m_files[idx].slotIndex;
        if (!(mode & QIODevice::Truncate) && m_files[idx].usedSize > 0)
            data = readBlobFromDisk(m_files[idx]);
    } else {
        // ---- New file ----
        if (!(mode & QIODevice::WriteOnly))
            return nullptr;   // cannot create without write intent

        QByteArray nameUtf8 = filename.toUtf8();
        if (nameUtf8.size() > MAX_FILENAME_BYTES) {
            qCWarning(EVFS) << "Filename too long (max" << MAX_FILENAME_BYTES
                            << "UTF-8 bytes):" << filename;
            return nullptr;
        }

        slotIndex = findFreeSlot();
        if (slotIndex < 0) {
            qCWarning(EVFS) << "No free header slots (max" << SLOT_COUNT << ")";
            return nullptr;
        }

        FilePointer fp;
        fp.id            = QUuid::createUuid();
        fp.filename      = filename;
        fp.dataOffset    = allocateBlock(DATA_ALIGNMENT);
        fp.allocatedSize = DATA_ALIGNMENT;
        fp.usedSize      = 0;
        fp.flags         = 0;
        fp.slotIndex     = slotIndex;

        if (!encryptSlot(slotIndex, fp))
            return nullptr;

        m_files.append(fp);
    }

    auto* vf = new VirtualFile(this, slotIndex, data);

    // QBuffer doesn't recognise Truncate — strip it before open()
    const QIODevice::OpenMode bufMode = mode & ~QIODevice::Truncate;
    if (!vf->open(bufMode)) {
        delete vf;
        secureWipe(data);
        return nullptr;
    }

    registerOpenFile(vf);
    secureWipe(data);
    return vf;
}

QByteArray EncryptedVFSContainer::readFile(const QString& filename)
{
    if (!m_isOpen)
        return {};
    const int idx = findFileSlot(filename);
    if (idx < 0 || m_files[idx].usedSize == 0)
        return {};
    return readBlobFromDisk(m_files[idx]);
}

bool EncryptedVFSContainer::writeFileData(int slotIndex,
                                           const QByteArray& plaintext)
{
    int fileIdx = -1;
    for (int i = 0; i < m_files.size(); ++i) {
        if (m_files[i].slotIndex == slotIndex) {
            fileIdx = i;
            break;
        }
    }
    if (fileIdx < 0)
        return false;

    FilePointer& fp = m_files[fileIdx];

    if (plaintext.isEmpty()) {
        fp.usedSize = 0;
        return encryptSlot(slotIndex, fp);
    }

    // Check whether the current allocation can hold the new blob
    const quint64 blobSize = static_cast<quint64>(NONCE_SIZE)
                           + static_cast<quint64>(plaintext.size())
                           + MAC_SIZE;
    if (blobSize > fp.allocatedSize) {
        const quint64 newAlloc = nextPowerOf2(blobSize);
        fp.dataOffset    = allocateBlock(newAlloc);
        fp.allocatedSize = newAlloc;
    }

    fp.usedSize = static_cast<quint64>(plaintext.size());

    if (!writeBlobToDisk(fp, plaintext))
        return false;

    return encryptSlot(slotIndex, fp);
}

bool EncryptedVFSContainer::deleteFile(const QString& filename)
{
    const int idx = findFileSlot(filename);
    if (idx < 0)
        return false;

    if (!clearSlot(m_files[idx].slotIndex))
        return false;

    m_files.removeAt(idx);
    return true;
}

bool EncryptedVFSContainer::renameFile(const QString& oldName,
                                        const QString& newName)
{
    const int idx = findFileSlot(oldName);
    if (idx < 0)
        return false;

    if (newName.toUtf8().size() > MAX_FILENAME_BYTES)
        return false;

    m_files[idx].filename = newName;
    return encryptSlot(m_files[idx].slotIndex, m_files[idx]);
}

quint64 EncryptedVFSContainer::containerSize() const
{
    return static_cast<quint64>(m_file.size());
}

// ---------------------------------------------------------------------------
// Container creation
// ---------------------------------------------------------------------------

bool EncryptedVFSContainer::createContainer(const QString& path,
                                             quint64 maxSize)
{
    QStorageInfo storage(QFileInfo(path).absolutePath());
    const quint64 freeSpace = static_cast<quint64>(storage.bytesAvailable());

    const quint64 minSize = DATA_START + 1024 * 1024;   // header + 1 MiB data
    const quint64 ceiling = (maxSize > 0)
                          ? qMin(maxSize, freeSpace / 2)
                          : qMin(freeSpace / 2,
                                 quint64(4ULL * 1024 * 1024 * 1024));

    if (ceiling < minSize) {
        qCCritical(EVFS) << "Not enough disk space to create container";
        return false;
    }

    // Randomise size within [minSize, ceiling]
    quint64 offset = 0;
    const quint64 range = ceiling - minSize;
    if (range > 0) {
        randombytes_buf(&offset, sizeof(offset));
        offset %= range;
    }
    quint64 containerSize = ((minSize + offset + DATA_ALIGNMENT - 1)
                              / DATA_ALIGNMENT) * DATA_ALIGNMENT;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qCCritical(EVFS) << "Cannot create file:" << path;
        return false;
    }

    if (!file.resize(static_cast<qint64>(containerSize))) {
        qCCritical(EVFS) << "Cannot resize container to" << containerSize;
        file.close();
        QFile::remove(path);
        return false;
    }

    // Fill the entire file with cryptographically secure random bytes.
    // After this step every bit looks random — slots, data area, everything.
    constexpr quint64 CHUNK = 1024 * 1024;
    QByteArray chunk(static_cast<int>(CHUNK), Qt::Uninitialized);
    quint64 remaining = containerSize;
    while (remaining > 0) {
        const quint64 n = qMin(remaining, CHUNK);
        randombytes_buf(chunk.data(), static_cast<size_t>(n));
        if (file.write(chunk.constData(), static_cast<qint64>(n))
                != static_cast<qint64>(n)) {
            qCCritical(EVFS) << "Write error during container creation";
            file.close();
            QFile::remove(path);
            secureWipe(chunk);
            return false;
        }
        remaining -= n;
    }
    secureWipe(chunk);

    // Overwrite the first bytes with the public magic header
    file.seek(0);
    file.write(MAGIC, MAGIC_SIZE);

    // Generate and write the global salt
    QByteArray salt(static_cast<int>(SALT_SIZE), Qt::Uninitialized);
    randombytes_buf(salt.data(), SALT_SIZE);
    file.write(salt);

    file.flush();
    file.close();
    secureWipe(salt);

    qCDebug(EVFS) << "Container created:" << path << "size:" << containerSize;
    return true;
}

// ---------------------------------------------------------------------------
// Key derivation
// ---------------------------------------------------------------------------

bool EncryptedVFSContainer::deriveKey(const QString& passphrase,
                                      const QByteArray& salt)
{
    m_key.resize(KEY_SIZE);
    QByteArray pw = passphrase.toUtf8();

    const int rc = crypto_pwhash(
        reinterpret_cast<unsigned char*>(m_key.data()),
        KEY_SIZE,
        pw.constData(),
        static_cast<unsigned long long>(pw.size()),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        KDF_OPSLIMIT,
        KDF_MEMLIMIT,
        crypto_pwhash_ALG_ARGON2ID13);

    secureWipe(pw);

    if (rc != 0) {
        secureWipe(m_key);
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Slot header I/O
// ---------------------------------------------------------------------------

bool EncryptedVFSContainer::loadSlots()
{
    m_files.clear();

    for (quint32 i = 0; i < SLOT_COUNT; ++i) {
        const quint64 off = HEADER_OFFSET + static_cast<quint64>(i) * SLOT_SIZE;
        m_file.seek(static_cast<qint64>(off));
        QByteArray raw = m_file.read(SLOT_SIZE);
        if (raw.size() != static_cast<int>(SLOT_SIZE))
            continue;

        QByteArray nonce = raw.left(NONCE_SIZE);
        QByteArray ct    = raw.mid(NONCE_SIZE);   // SLOT_PLAINTEXT_SIZE + MAC_SIZE = 104 B

        // Additional data: 4-byte LE slot index (binds ciphertext to position)
        quint32 idxLE = qToLittleEndian(i);
        QByteArray ad(reinterpret_cast<const char*>(&idxLE), sizeof(idxLE));

        QByteArray pt(SLOT_PLAINTEXT_SIZE, 0);
        unsigned long long ptLen = 0;

        if (crypto_aead_xchacha20poly1305_ietf_decrypt(
                reinterpret_cast<unsigned char*>(pt.data()), &ptLen,
                nullptr,
                reinterpret_cast<const unsigned char*>(ct.constData()),
                static_cast<unsigned long long>(ct.size()),
                reinterpret_cast<const unsigned char*>(ad.constData()),
                static_cast<unsigned long long>(ad.size()),
                reinterpret_cast<const unsigned char*>(nonce.constData()),
                reinterpret_cast<const unsigned char*>(m_key.constData())) == 0)
        {
            FilePointer fp;
            if (deserializeFilePointer(pt, fp)) {
                fp.slotIndex = static_cast<int>(i);
                m_files.append(fp);
            }
        }

        secureWipe(pt);
        secureWipe(nonce);
        secureWipe(ct);
        secureWipe(raw);
    }

    return true;
}

bool EncryptedVFSContainer::encryptSlot(int slotIndex, const FilePointer& fp)
{
    QByteArray pt = serializeFilePointer(fp);

    QByteArray nonce(NONCE_SIZE, Qt::Uninitialized);
    randombytes_buf(nonce.data(), NONCE_SIZE);

    quint32 idxLE = qToLittleEndian(static_cast<quint32>(slotIndex));
    QByteArray ad(reinterpret_cast<const char*>(&idxLE), sizeof(idxLE));

    // ciphertext = plaintext + MAC  →  88 + 16 = 104 bytes
    QByteArray ct(SLOT_PLAINTEXT_SIZE + MAC_SIZE, 0);
    unsigned long long ctLen = 0;

    if (crypto_aead_xchacha20poly1305_ietf_encrypt(
            reinterpret_cast<unsigned char*>(ct.data()), &ctLen,
            reinterpret_cast<const unsigned char*>(pt.constData()),
            SLOT_PLAINTEXT_SIZE,
            reinterpret_cast<const unsigned char*>(ad.constData()),
            static_cast<unsigned long long>(ad.size()),
            nullptr,
            reinterpret_cast<const unsigned char*>(nonce.constData()),
            reinterpret_cast<const unsigned char*>(m_key.constData())) != 0)
    {
        secureWipe(pt);
        return false;
    }
    secureWipe(pt);

    // nonce(24) + ct(104) = 128 = SLOT_SIZE — no extra padding needed
    const quint64 off = HEADER_OFFSET + static_cast<quint64>(slotIndex) * SLOT_SIZE;
    m_file.seek(static_cast<qint64>(off));
    m_file.write(nonce);
    m_file.write(ct.constData(), static_cast<qint64>(ctLen));
    m_file.flush();

    secureWipe(nonce);
    secureWipe(ct);
    return true;
}

bool EncryptedVFSContainer::clearSlot(int slotIndex)
{
    QByteArray rnd(static_cast<int>(SLOT_SIZE), Qt::Uninitialized);
    randombytes_buf(rnd.data(), SLOT_SIZE);

    const quint64 off = HEADER_OFFSET + static_cast<quint64>(slotIndex) * SLOT_SIZE;
    m_file.seek(static_cast<qint64>(off));
    const bool ok = (m_file.write(rnd) == static_cast<qint64>(SLOT_SIZE));
    m_file.flush();

    secureWipe(rnd);
    return ok;
}

// ---------------------------------------------------------------------------
// FilePointer serialisation
// ---------------------------------------------------------------------------

QByteArray EncryptedVFSContainer::serializeFilePointer(const FilePointer& fp) const
{
    QByteArray buf(SLOT_PLAINTEXT_SIZE, '\0');

    // [0..15]  UUID
    QByteArray uuid = fp.id.toRfc4122();
    memcpy(buf.data(), uuid.constData(), 16);

    // [16]     filename length
    QByteArray name = fp.filename.toUtf8().left(MAX_FILENAME_BYTES);
    buf[16] = static_cast<char>(name.size());

    // [17..62] filename UTF-8 (zero-padded to 46 bytes)
    memcpy(buf.data() + 17, name.constData(), static_cast<size_t>(name.size()));

    // Helper: write a 64-bit little-endian value
    auto put64 = [&](int offset, quint64 v) {
        v = qToLittleEndian(v);
        memcpy(buf.data() + offset, &v, 8);
    };

    put64(63, fp.dataOffset);
    put64(71, fp.allocatedSize);
    put64(79, fp.usedSize);
    buf[87] = static_cast<char>(fp.flags);

    return buf;
}

bool EncryptedVFSContainer::deserializeFilePointer(const QByteArray& buf,
                                                    FilePointer& fp) const
{
    if (buf.size() < SLOT_PLAINTEXT_SIZE)
        return false;

    fp.id = QUuid::fromRfc4122(QByteArrayView(buf.constData(), 16));

    const quint8 nameLen = static_cast<quint8>(buf[16]);
    if (nameLen > MAX_FILENAME_BYTES)
        return false;
    fp.filename = QString::fromUtf8(buf.constData() + 17, nameLen);

    auto get64 = [&](int offset) -> quint64 {
        quint64 v;
        memcpy(&v, buf.constData() + offset, 8);
        return qFromLittleEndian(v);
    };

    fp.dataOffset    = get64(63);
    fp.allocatedSize = get64(71);
    fp.usedSize      = get64(79);
    fp.flags         = static_cast<quint8>(buf[87]);

    return true;
}

// ---------------------------------------------------------------------------
// Encrypted blob I/O (file data area)
// ---------------------------------------------------------------------------

QByteArray EncryptedVFSContainer::readBlobFromDisk(const FilePointer& fp)
{
    if (fp.usedSize == 0)
        return {};

    m_file.seek(static_cast<qint64>(fp.dataOffset));

    QByteArray nonce = m_file.read(NONCE_SIZE);
    if (nonce.size() != NONCE_SIZE)
        return {};

    const quint64 ctSize = fp.usedSize + MAC_SIZE;
    QByteArray ct = m_file.read(static_cast<qint64>(ctSize));
    if (static_cast<quint64>(ct.size()) != ctSize) {
        secureWipe(nonce);
        return {};
    }

    // Additional data: file UUID (binds blob to its slot identity)
    QByteArray ad = fp.id.toRfc4122();

    QByteArray pt(static_cast<int>(fp.usedSize), 0);
    unsigned long long ptLen = 0;

    if (crypto_aead_xchacha20poly1305_ietf_decrypt(
            reinterpret_cast<unsigned char*>(pt.data()), &ptLen,
            nullptr,
            reinterpret_cast<const unsigned char*>(ct.constData()),
            static_cast<unsigned long long>(ct.size()),
            reinterpret_cast<const unsigned char*>(ad.constData()),
            static_cast<unsigned long long>(ad.size()),
            reinterpret_cast<const unsigned char*>(nonce.constData()),
            reinterpret_cast<const unsigned char*>(m_key.constData())) != 0)
    {
        qCWarning(EVFS) << "Blob decryption failed for" << fp.filename;
        secureWipe(pt);
        secureWipe(ct);
        secureWipe(nonce);
        return {};
    }

    secureWipe(ct);
    secureWipe(nonce);
    return pt;
}

bool EncryptedVFSContainer::writeBlobToDisk(FilePointer& fp,
                                             const QByteArray& plaintext)
{
    QByteArray nonce(NONCE_SIZE, Qt::Uninitialized);
    randombytes_buf(nonce.data(), NONCE_SIZE);

    QByteArray ad = fp.id.toRfc4122();

    QByteArray ct(plaintext.size() + MAC_SIZE, 0);
    unsigned long long ctLen = 0;

    if (crypto_aead_xchacha20poly1305_ietf_encrypt(
            reinterpret_cast<unsigned char*>(ct.data()), &ctLen,
            reinterpret_cast<const unsigned char*>(plaintext.constData()),
            static_cast<unsigned long long>(plaintext.size()),
            reinterpret_cast<const unsigned char*>(ad.constData()),
            static_cast<unsigned long long>(ad.size()),
            nullptr,
            reinterpret_cast<const unsigned char*>(nonce.constData()),
            reinterpret_cast<const unsigned char*>(m_key.constData())) != 0)
    {
        secureWipe(nonce);
        return false;
    }

    m_file.seek(static_cast<qint64>(fp.dataOffset));
    m_file.write(nonce);
    m_file.write(ct.constData(), static_cast<qint64>(ctLen));

    // Fill the rest of the allocated block with random (hides actual size)
    const quint64 written = static_cast<quint64>(NONCE_SIZE) + ctLen;
    if (written < fp.allocatedSize) {
        const quint64 padSize = fp.allocatedSize - written;
        QByteArray pad(static_cast<int>(padSize), Qt::Uninitialized);
        randombytes_buf(pad.data(), static_cast<size_t>(padSize));
        m_file.write(pad);
        secureWipe(pad);
    }

    m_file.flush();
    secureWipe(nonce);
    secureWipe(ct);
    return true;
}

// ---------------------------------------------------------------------------
// Block allocation & container growth
// ---------------------------------------------------------------------------

quint64 EncryptedVFSContainer::allocateBlock(quint64 requiredSize)
{
    // Simple strategy: allocate at the end of the currently-used data area.
    // Old (now-unused) blocks become random-looking garbage.
    quint64 endOfData = DATA_START;
    for (const auto& fp : m_files) {
        const quint64 end = fp.dataOffset + fp.allocatedSize;
        if (end > endOfData)
            endOfData = end;
    }

    const quint64 newOffset = ((endOfData + DATA_ALIGNMENT - 1)
                                / DATA_ALIGNMENT) * DATA_ALIGNMENT;

    const quint64 requiredEnd = newOffset + requiredSize;
    if (requiredEnd > static_cast<quint64>(m_file.size()))
        growContainer(requiredEnd);

    return newOffset;
}

void EncryptedVFSContainer::growContainer(quint64 requiredEnd)
{
    // Add random padding 0–4 MiB to hide exact allocation pattern
    quint32 padRaw;
    randombytes_buf(&padRaw, sizeof(padRaw));
    const quint64 padding = padRaw % (4 * 1024 * 1024);

    quint64 newSize = ((requiredEnd + padding + DATA_ALIGNMENT - 1)
                        / DATA_ALIGNMENT) * DATA_ALIGNMENT;

    const quint64 oldSize = static_cast<quint64>(m_file.size());
    m_file.resize(static_cast<qint64>(newSize));

    // Fill newly appended bytes with random
    m_file.seek(static_cast<qint64>(oldSize));
    constexpr quint64 CHUNK = 1024 * 1024;
    QByteArray buf(static_cast<int>(CHUNK), Qt::Uninitialized);
    quint64 remaining = newSize - oldSize;
    while (remaining > 0) {
        const quint64 n = qMin(remaining, CHUNK);
        randombytes_buf(buf.data(), static_cast<size_t>(n));
        m_file.write(buf.constData(), static_cast<qint64>(n));
        remaining -= n;
    }
    secureWipe(buf);
    m_file.flush();

    emit containerGrown(newSize);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

int EncryptedVFSContainer::findFileSlot(const QString& filename) const
{
    for (int i = 0; i < m_files.size(); ++i)
        if (m_files[i].filename == filename)
            return i;
    return -1;
}

int EncryptedVFSContainer::findFreeSlot() const
{
    QSet<int> used;
    used.reserve(m_files.size());
    for (const auto& fp : m_files)
        used.insert(fp.slotIndex);

    for (int i = 0; i < static_cast<int>(SLOT_COUNT); ++i)
        if (!used.contains(i))
            return i;
    return -1;
}

quint64 EncryptedVFSContainer::nextPowerOf2(quint64 v)
{
    if (v <= DATA_ALIGNMENT)
        return DATA_ALIGNMENT;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    return v + 1;
}

void EncryptedVFSContainer::secureWipe(QByteArray& data)
{
    if (!data.isEmpty())
        sodium_memzero(data.data(), static_cast<size_t>(data.size()));
    data.clear();
}

void EncryptedVFSContainer::registerOpenFile(VirtualFile* file)
{
    m_openFiles.insert(file);
}

void EncryptedVFSContainer::unregisterOpenFile(VirtualFile* file)
{
    m_openFiles.remove(file);
}
