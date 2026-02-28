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

#ifndef VIRTUALFILE_H
#define VIRTUALFILE_H

#include <QBuffer>

class EncryptedVFSContainer;

/**
 * @brief In-memory virtual file backed by an encrypted VFS container.
 *
 * VirtualFile is a QBuffer subclass that holds decrypted file content in
 * RAM.  On close() or flush() the content is re-encrypted with a fresh
 * nonce and written back to the container.  The underlying buffer is
 * securely wiped (overwritten with zeros via sodium_memzero) on destruction.
 *
 * @note Ownership: the caller owns the VirtualFile (delete it when done, or
 *       parent it to a QObject).
 * @note Thread safety: use from one thread only, same as the container.
 */
class VirtualFile : public QBuffer
{
    Q_OBJECT

public:
    /**
     * @param container  Owning container (used for write-back on flush).
     * @param slotIndex  Slot index inside the container header.
     * @param data       Decrypted initial content (may be empty for new files).
     * @param parent     Optional QObject parent.
     */
    VirtualFile(EncryptedVFSContainer* container, int slotIndex, const QByteArray& data, QObject* parent = nullptr);
    ~VirtualFile() override;

    /**
     * @brief Flush dirty data and close the device.
     *
     * Re-encrypts the buffer if modified, writes it back to the container,
     * and then calls QBuffer::close().
     */
    void close() override;

    /**
     * @brief Re-encrypt and write the buffer back to the container now.
     * @return true on success (or if nothing to flush).
     */
    bool flush();

    int slotIndex() const { return m_slotIndex; }

protected:
    qint64 writeData(const char* data, qint64 len) override;

private:
    EncryptedVFSContainer* m_container;
    int m_slotIndex;
    bool m_dirty;
};

#endif // VIRTUALFILE_H
