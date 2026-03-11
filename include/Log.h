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

#ifndef LOG_H
#define LOG_H

#include <QString>

#include "LogManager.h"

extern Log* LOGGER;

// ---------------------------------------------------------------------------
// Macros: use these so __FILE__ and __LINE__ are recorded for debug output.
// ---------------------------------------------------------------------------
#define LOG_D() (LOGGER->d(__FILE__, __LINE__))
#define LOG_I() (LOGGER->i(__FILE__, __LINE__))
#define LOG_W() (LOGGER->w(__FILE__, __LINE__))
#define LOG_C() (LOGGER->c(__FILE__, __LINE__))
#define LOG_F() (LOGGER->f(__FILE__, __LINE__))

#endif // LOG_H
