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

#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QString>

#include "LogLevel.h"
#include "LogStream.h"

class UiPluginInterface;

/**
 * @brief Log chain object: each plugin gets a named Log that forwards
 * to a parent until the root, which sends messages to the active UI plugin.
 *
 * This keeps all log output under UI control (e.g. ui-console can write
 * through the terminal instead of raw stdout so the prompt is not broken).
 * Use LOG_D() << "msg" << value; etc. for streaming; file/line are recorded
 * for debug builds.
 */
class Log
{
public:
    Log(const QString& name, LogLevel verbosity, Log* parent);
    ~Log();

    /**
     * @brief Set the log sink for the root log (the active UI plugin).
     *
     * Messages are delivered to this sink when the root log receives them.
     * Call after the UI plugin is activated (e.g. from main).
     *
     * @param sink UI plugin that implements log output; nullptr to use Qt default (qDebug etc.).
     */
    void setSink(UiPluginInterface* sink);

    /**
     * @brief Return a debug-level stream with optional file/line (for macros).
     * @param file Optional __FILE__ (nullptr to omit).
     * @param line Optional __LINE__ (0 to omit).
     */
    LogStream d(const char* file = nullptr, int line = 0);

    /**
     * @brief Return an info-level stream with optional file/line.
     */
    LogStream i(const char* file = nullptr, int line = 0);

    /**
     * @brief Return a warning-level stream with optional file/line.
     */
    LogStream w(const char* file = nullptr, int line = 0);

    /**
     * @brief Return a critical-level stream with optional file/line.
     */
    LogStream c(const char* file = nullptr, int line = 0);

    /**
     * @brief Return a fatal-level stream with optional file/line.
     */
    LogStream f(const char* file = nullptr, int line = 0);

    /**
     * @brief Return the parent log in the chain, or nullptr if this is the root.
     */
    Log* parent() const { return m_parent; }

    /**
     * @brief Forward a composed message to the parent or to the sink (root).
     *
     * Nested logs prepend their name; the root sends to the UI plugin or Qt default.
     *
     * @param level Log level.
     * @param msg   Composed message string.
     * @param file  Optional source file (for debug display).
     * @param line  Optional source line (for debug display).
     */
    void output(LogLevel level, const QString& msg, const char* file = nullptr, int line = 0);

    /** @brief Find the root log (no parent) and set its sink. */
    Log* root();

private:
    QString m_name;
    LogLevel m_verbosity;
    Log* m_parent = nullptr;

    UiPluginInterface* m_sink = nullptr;
};

#endif // LOGMANAGER_H
