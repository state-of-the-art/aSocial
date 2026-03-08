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

#include "Spinner.h"
#include "Style.h"
#include "Terminal.h"

#include <chrono>

// ===========================================================================
// Spinner
// ===========================================================================

Spinner::Spinner(Terminal& term)
    : m_term(term)
{}

Spinner::~Spinner()
{
    if( m_running.load() )
        stop(false);
}

void Spinner::start(const QString& message)
{
    if( m_running.load() )
        return;
    m_message = message;
    m_running.store(true);
    m_term.hideCursor();
    m_thread = std::thread(&Spinner::animationLoop, this);
}

void Spinner::stop(bool success, const QString& message)
{
    if( !m_running.load() )
        return;

    m_running.store(false);
    if( m_thread.joinable() )
        m_thread.join();

    QString finalMsg = message.isEmpty() ? m_message : message;

    m_term.clearLine();
    if( success ) {
        m_term.writeLine(
            QStringLiteral("  %1%2%3 %4").arg(Style::success(), Style::checkmark, Style::reset(), finalMsg));
    } else {
        m_term.writeLine(QStringLiteral("  %1%2%3 %4").arg(Style::error(), Style::cross, Style::reset(), finalMsg));
    }
    m_term.showCursor();
}

void Spinner::animationLoop()
{
    int frame = 0;
    while( m_running.load() ) {
        m_term.clearLine();
        m_term.write(
            QStringLiteral("  %1%2%3 %4").arg(Style::accent(), Style::spinnerFrames[frame], Style::reset(), m_message));

        frame = (frame + 1) % Style::spinnerFrameCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

// ===========================================================================
// ProgressBar
// ===========================================================================

ProgressBar::ProgressBar(Terminal& term, const QString& label, int width)
    : m_term(term)
    , m_label(label)
    , m_width(width)
{}

void ProgressBar::update(int percent)
{
    percent = qBound(0, percent, 100);
    if( percent == m_lastPercent )
        return;
    m_lastPercent = percent;

    int filled = m_width * percent / 100;
    int empty = m_width - filled;

    QString bar = QString(filled, QChar(0x2588))   // █
                  + QString(empty, QChar(0x2591)); // ░

    m_term.clearLine();
    m_term.write(QStringLiteral("  %1: %2%3%4 %5%").arg(m_label, Style::accent(), bar, Style::reset()).arg(percent));
}

void ProgressBar::finish(bool success)
{
    update(100);
    m_term.writeLine();
    if( success ) {
        m_term.writeLine(
            QStringLiteral("  %1%2%3 %4 complete").arg(Style::success(), Style::checkmark, Style::reset(), m_label));
    } else {
        m_term.writeLine(
            QStringLiteral("  %1%2%3 %4 failed").arg(Style::error(), Style::cross, Style::reset(), m_label));
    }
}
