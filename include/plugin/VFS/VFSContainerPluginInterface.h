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

#ifndef VFSCONTAINERPLUGININTERFACE_H
#define VFSCONTAINERPLUGININTERFACE_H

// Encrypted container interface – represents one opened VFS container.
// Returned by VFSPluginInterface::openContainer().

#include <QIODevice>
#include <QStringList>

#define VFSContainerPluginInterface_iid "io.stateoftheart.asocial.plugin.VFSContainerPluginInterface"

/**
 * @brief Handle to an opened encrypted container.
 *
 * Obtained via VFSPluginInterface::openContainer().  Provides file-level
 * access to the virtual files stored inside the container.  Each virtual
 * file is returned as a standard QIODevice, so any Qt component that works
 * with QIODevice (SQL drivers, image loaders, …) can consume it directly.
 *
 * Ownership: the caller owns the returned pointer.  Call close() and then
 * @c delete (or parent the object to a QObject for automatic cleanup).
 *
 * Container format (public – security by cryptography only):
 * @code
 *   Offset  0          : Magic "VFSENCv1"    (8 bytes)
 *   Offset  8          : Global salt          (16 bytes)
 *   Offset 24          : 4096 slots × 128 B = 512 KiB header
 *   After header (4 KiB-aligned): Data blobs (encrypted file contents)
 * @endcode
 *
 * @note Thread safety: NOT thread-safe.  Use from a single thread, or
 *       protect every call with an external mutex.
 */
class VFSContainerPluginInterface
{
public:
    virtual ~VFSContainerPluginInterface() {}

    /**
     * @brief Return the sub-interface type identifier.
     */
    static QLatin1String type() { return QLatin1String(VFSContainerPluginInterface_iid); }

    /**
     * @brief List virtual filenames visible under the key that opened
     *        this container.
     */
    virtual QStringList listFiles() const = 0;

    /**
     * @brief Open (or create) a virtual file inside the container.
     *
     * Returns a memory-backed QIODevice (QBuffer subclass).  On close() or
     * the custom flush() the content is re-encrypted with a fresh nonce and
     * written back.  No temporary files are created on disk.
     *
     * Ownership of the device is transferred to the caller.
     *
     * @param filename  Virtual filename (max 46 UTF-8 bytes).
     * @param mode      Qt open-mode flags (default ReadWrite).
     * @return QIODevice*, or @c nullptr on error.
     */
    virtual QIODevice* openFile(const QString& filename,
                                QIODevice::OpenMode mode = QIODevice::ReadWrite) = 0;

    /**
     * @brief Convenience: read the full decrypted content of a virtual file.
     */
    virtual QByteArray readFile(const QString& filename) = 0;

    /**
     * @brief Delete a virtual file (slot overwritten with random).
     */
    virtual bool deleteFile(const QString& filename) = 0;

    /**
     * @brief Rename a virtual file (metadata-only in the header slot).
     */
    virtual bool renameFile(const QString& oldName,
                            const QString& newName) = 0;

    /**
     * @brief Flush all open files and close the container.
     */
    virtual void close() = 0;

    /**
     * @brief Whether the container is currently open.
     */
    virtual bool isOpen() const = 0;

signals:
    /** Emitted when the container auto-extends (optional, cast to QObject*
     *  to connect). */
    virtual void containerGrown(quint64 newSize) = 0;
};

Q_DECLARE_INTERFACE(VFSContainerPluginInterface, VFSContainerPluginInterface_iid)

#endif // VFSCONTAINERPLUGININTERFACE_H
