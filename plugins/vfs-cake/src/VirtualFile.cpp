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

#include "VirtualFile.h"
#include "EncryptedVFSContainer.h"

#include <sodium.h>

VirtualFile::VirtualFile(EncryptedVFSContainer* container, int slotIndex,
                         const QByteArray& data, QObject* parent)
    : QBuffer(parent)
    , m_container(container)
    , m_slotIndex(slotIndex)
    , m_dirty(false)
{
    setData(data);
}

VirtualFile::~VirtualFile()
{
    if (isOpen())
        VirtualFile::close();

    // Secure-wipe the internal buffer before QBuffer frees it
    QByteArray& buf = buffer();
    if (!buf.isEmpty())
        sodium_memzero(buf.data(), static_cast<size_t>(buf.size()));
}

void VirtualFile::close()
{
    flush();

    if (m_container) {
        m_container->unregisterOpenFile(this);
        m_container = nullptr;
    }

    QBuffer::close();
}

bool VirtualFile::flush()
{
    if (!m_dirty || !m_container || !m_container->isOpen())
        return !m_dirty;

    bool ok = m_container->writeFileData(m_slotIndex, data());
    if (ok)
        m_dirty = false;
    return ok;
}

qint64 VirtualFile::writeData(const char* data, qint64 len)
{
    m_dirty = true;
    return QBuffer::writeData(data, len);
}
