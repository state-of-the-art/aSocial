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

#ifndef ENCRYPTEDVFSCONTAINER_H
#define ENCRYPTEDVFSCONTAINER_H

#include <QObject>
#include <QFile>
#include <QByteArray>
#include <QVector>
#include <QSet>
#include <QString>
#include <QUuid>

#include "plugin/VFS/VFSContainerPluginInterface.h"

class VirtualFile;

/**
 * @brief Concrete implementation of VFSContainerPluginInterface.
 *
 * Manages a single *.vfs file with a fixed-size header of 4096 encrypted
 * slots (inspired by Shufflecake).  Each slot may hold a FilePointer that
 * describes a virtual file stored in the data area.  Empty and occupied
 * slots are indistinguishable without the correct key, providing plausible
 * deniability.
 *
 * Crypto primitives (libsodium):
 *   - Key derivation : Argon2id  (crypto_pwhash, OPSLIMIT=4, MEMLIMIT=128 MiB)
 *   - AEAD           : XChaCha20-Poly1305
 *   - Random         : randombytes_buf
 *
 * @note Thread safety: NOT thread-safe.  Use from one thread only, or
 *       protect all calls with an external mutex.
 */
class EncryptedVFSContainer : public QObject, public VFSContainerPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(VFSContainerPluginInterface)

public:
    explicit EncryptedVFSContainer(QObject* parent = nullptr);
    ~EncryptedVFSContainer() override;

    EncryptedVFSContainer(const EncryptedVFSContainer&) = delete;
    EncryptedVFSContainer& operator=(const EncryptedVFSContainer&) = delete;

    /**
     * @brief Open or create the encrypted container.
     *
     * Called by the plugin factory; not part of the public container interface.
     *
     * @param containerPath  Path to the *.vfs file.
     * @param passphrase     User passphrase (empty = demo mode).
     * @param maxContainerSize  Cap on new container size (0 = up to 4 GiB).
     * @return true on success.
     */
    bool open(const QString& containerPath, const QString& passphrase,
              quint64 maxContainerSize = 0);

    // ---- VFSContainerPluginInterface ----
    QStringList listFiles() const override;
    QIODevice*  openFile(const QString& filename,
                         QIODevice::OpenMode mode = QIODevice::ReadWrite) override;
    QByteArray  readFile(const QString& filename) override;
    bool deleteFile(const QString& filename) override;
    bool renameFile(const QString& oldName, const QString& newName) override;
    void close() override;
    bool isOpen() const override { return m_isOpen; }

    /**
     * @brief Write encrypted file data back into the container blob area.
     *
     * Called by VirtualFile on flush/close.  Re-encrypts @p plaintext with
     * a fresh nonce and writes it to the slot's data block (reallocating if
     * the block is too small).  Updates the slot header accordingly.
     */
    bool writeFileData(int slotIndex, const QByteArray& plaintext);

    void registerOpenFile(VirtualFile* file);
    void unregisterOpenFile(VirtualFile* file);

    quint64 containerSize() const;

signals:
    void containerGrown(quint64 newSize) override;

public:
    // ----- On-disk layout constants (all values in bytes) -----

    static constexpr char     MAGIC[]          = "VFSCAKE1";
    static constexpr quint64  MAGIC_SIZE       = 8;
    static constexpr quint64  SALT_SIZE        = 16;   // crypto_pwhash_SALTBYTES
    static constexpr quint64  SLOT_COUNT       = 4096;
    static constexpr quint64  SLOT_SIZE        = 128;
    static constexpr quint64  HEADER_OFFSET    = MAGIC_SIZE + SALT_SIZE;        // 24
    static constexpr quint64  HEADER_SIZE      = SLOT_COUNT * SLOT_SIZE;        // 524 288
    static constexpr quint64  TOTAL_HEADER     = HEADER_OFFSET + HEADER_SIZE;   // 524 312
    static constexpr quint64  DATA_ALIGNMENT   = 4096;
    static constexpr quint64  DATA_START       = ((TOTAL_HEADER + DATA_ALIGNMENT - 1)
                                                   / DATA_ALIGNMENT) * DATA_ALIGNMENT;

    // ----- Crypto constants -----

    static constexpr int KEY_SIZE             = 32;
    static constexpr int NONCE_SIZE           = 24;
    static constexpr int MAC_SIZE             = 16;
    static constexpr int SLOT_PLAINTEXT_SIZE  = static_cast<int>(SLOT_SIZE) - NONCE_SIZE - MAC_SIZE; // 88
    static constexpr int MAX_FILENAME_BYTES   = 46;

    static constexpr unsigned long long KDF_OPSLIMIT = 4;
    static constexpr size_t             KDF_MEMLIMIT = 128ULL * 1024 * 1024;

private:
    /**
     * Serialised layout (88 bytes, little-endian):
     *   [0..15]   UUID (RFC 4122 binary)
     *   [16]      filename length N
     *   [17..62]  filename UTF-8 (zero-padded, 46 bytes)
     *   [63..70]  dataOffset   (uint64)
     *   [71..78]  allocatedSize(uint64)
     *   [79..86]  usedSize     (uint64)
     *   [87]      flags        (uint8)
     */
    struct FilePointer {
        QUuid   id;
        QString filename;
        quint64 dataOffset     = 0;
        quint64 allocatedSize  = 0;
        quint64 usedSize       = 0;
        quint8  flags          = 0;
        int     slotIndex      = -1;  // runtime-only (not serialised)
    };

    bool createContainer(const QString& path, quint64 maxSize);
    bool deriveKey(const QString& passphrase, const QByteArray& salt);
    bool loadSlots();
    bool encryptSlot(int slotIndex, const FilePointer& fp);
    bool clearSlot(int slotIndex);

    QByteArray serializeFilePointer(const FilePointer& fp) const;
    bool       deserializeFilePointer(const QByteArray& buf, FilePointer& fp) const;

    QByteArray readBlobFromDisk(const FilePointer& fp);
    bool       writeBlobToDisk(FilePointer& fp, const QByteArray& plaintext);

    quint64 allocateBlock(quint64 requiredSize);
    void    growContainer(quint64 requiredEnd);

    int findFileSlot(const QString& filename) const;
    int findFreeSlot() const;

    static quint64 nextPowerOf2(quint64 v);
    static void    secureWipe(QByteArray& data);

    QFile                m_file;
    QByteArray           m_key;
    QByteArray           m_salt;
    QVector<FilePointer> m_files;        // decoded slots for the current key
    QSet<VirtualFile*>   m_openFiles;    // tracked for container-wide flush
    bool                 m_isOpen = false;
};

#endif // ENCRYPTEDVFSCONTAINER_H
