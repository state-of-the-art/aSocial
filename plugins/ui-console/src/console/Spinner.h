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

#ifndef CONSOLE_SPINNER_H
#define CONSOLE_SPINNER_H

#include <atomic>
#include <thread>
#include <QString>

class Terminal;

/**
 * @brief Animated Braille-pattern spinner for long-running operations.
 *
 * Runs a background std::thread that cycles through the Unicode Braille
 * animation frames at ~80 ms intervals, producing a smooth "working…"
 * indicator.  Call start() before the blocking operation and stop()
 * when it finishes — stop() prints a colour-coded result line
 * (green ✓ / red ✗) to replace the spinner.
 *
 * Thread-safe: start/stop may be called from different threads.
 */
class Spinner
{
public:
    /**
     * @brief Construct a spinner bound to the given terminal.
     * @param term  Terminal used for output (non-owning).
     */
    explicit Spinner(Terminal& term);

    ~Spinner();

    Spinner(const Spinner&) = delete;
    Spinner& operator=(const Spinner&) = delete;

    /**
     * @brief Begin the animation with a descriptive message.
     *
     * The spinner is shown on the current line.  If a spinner is already
     * running, this call is a no-op.
     *
     * @param message  Text displayed next to the spinner glyph.
     */
    void start(const QString& message);

    /**
     * @brief Stop the animation and print a result line.
     *
     * @param success  true → green ✓, false → red ✗.
     * @param message  Final text to display (defaults to the start message).
     */
    void stop(bool success, const QString& message = {});

    /** @brief true while the animation thread is active. */
    bool isRunning() const { return m_running.load(); }

private:
    void animationLoop();

    Terminal& m_term;
    std::atomic<bool> m_running{false};
    std::thread m_thread;
    QString m_message;
};

/**
 * @brief Terminal progress bar for operations reporting 0-100 %.
 *
 * Unlike Spinner this is updated explicitly from the caller's thread
 * via update().  Call finish() to print the final state.
 */
class ProgressBar
{
public:
    /**
     * @brief Construct a progress bar.
     * @param term   Terminal used for output (non-owning).
     * @param label  Text shown before the bar.
     * @param width  Character width of the bar portion (default 30).
     */
    ProgressBar(Terminal& term, const QString& label, int width = 30);

    /**
     * @brief Redraw the bar at the given percentage.
     * @param percent  Progress value (clamped to 0-100).
     */
    void update(int percent);

    /**
     * @brief Print the final bar state (100 %) and move to a new line.
     * @param success  true → green ✓, false → red ✗.
     */
    void finish(bool success);

private:
    Terminal& m_term;
    QString m_label;
    int m_width;
    int m_lastPercent = -1;
};

#endif // CONSOLE_SPINNER_H
