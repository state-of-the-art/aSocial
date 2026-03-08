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

#ifndef CONSOLE_STYLE_H
#define CONSOLE_STYLE_H

#include <QString>

/**
 * @brief ANSI escape-code helpers for terminal colours, weights, and symbols.
 *
 * All constants are compile-time string literals that can be concatenated
 * freely with QString or embedded in raw C-string output.  Colour values
 * use the 256-colour palette so they work in virtually every modern
 * terminal emulator.
 *
 * Toggle all styling on/off at runtime with setEnabled().
 */
namespace Style {

inline bool& enabled()
{
    static bool s = true;
    return s;
}

/** @brief Enable or disable all ANSI styling globally. */
inline void setEnabled(bool on)
{
    enabled() = on;
}

/** @brief true when ANSI styling is active. */
inline bool isEnabled()
{
    return enabled();
}

/** @brief Return @p code unchanged if styling is on, empty string otherwise. */
inline const char* gate(const char* code)
{
    return enabled() ? code : "";
}

// --- Weight / decoration ---------------------------------------------------

inline const char* reset()
{
    return gate("\033[0m");
}
inline const char* bold()
{
    return gate("\033[1m");
}
inline const char* dim()
{
    return gate("\033[2m");
}
inline const char* italic()
{
    return gate("\033[3m");
}
inline const char* underline()
{
    return gate("\033[4m");
}

// --- Semantic colours (256-colour palette) ---------------------------------

inline const char* success()
{
    return gate("\033[38;5;82m");
}
inline const char* error()
{
    return gate("\033[38;5;196m");
}
inline const char* warning()
{
    return gate("\033[38;5;214m");
}
inline const char* info()
{
    return gate("\033[38;5;39m");
}
inline const char* muted()
{
    return gate("\033[38;5;245m");
}
inline const char* accent()
{
    return gate("\033[38;5;141m");
}
inline const char* highlight()
{
    return gate("\033[38;5;220m");
}
inline const char* cyan()
{
    return gate("\033[38;5;87m");
}

// --- Unicode symbols -------------------------------------------------------

constexpr const char* checkmark = "✓";
constexpr const char* cross = "✗";
constexpr const char* arrow = "›";
constexpr const char* bullet = "•";
constexpr const char* pointer = ">";
constexpr const char* dot = "·";

// --- Box-drawing (rounded corners) -----------------------------------------

constexpr const char* boxTL = "╭";
constexpr const char* boxTR = "╮";
constexpr const char* boxBL = "╰";
constexpr const char* boxBR = "╯";
constexpr const char* boxH = "─";
constexpr const char* boxV = "│";

// --- Spinner frames (Braille pattern) --------------------------------------

constexpr const char* spinnerFrames[] = {
    "\xe2\xa0\x8b", // ⠋
    "\xe2\xa0\x99", // ⠙
    "\xe2\xa0\xb9", // ⠹
    "\xe2\xa0\xb8", // ⠸
    "\xe2\xa0\xbc", // ⠼
    "\xe2\xa0\xb4", // ⠴
    "\xe2\xa0\xa6", // ⠦
    "\xe2\xa0\xa7", // ⠧
    "\xe2\xa0\x87", // ⠇
    "\xe2\xa0\x8f", // ⠏
};
constexpr int spinnerFrameCount = 10;

} // namespace Style

#endif // CONSOLE_STYLE_H
