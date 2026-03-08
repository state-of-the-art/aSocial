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

#include "CommandRouter.h"
#include "Style.h"
#include "Terminal.h"

CommandRouter::CommandRouter()
{
    m_root = std::make_shared<Node>();
    m_root->name = QStringLiteral("main");
    m_root->description = QStringLiteral("Main menu");
    m_current = m_root.get();
}

// ===========================================================================
// Tree construction
// ===========================================================================

CommandRouter::Node* CommandRouter::resolveMenu(const QString& path)
{
    if( path.isEmpty() )
        return m_root.get();

    Node* node = m_root.get();
    const QStringList parts = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for( const QString& part : parts ) {
        auto it = node->children.find(part);
        if( it == node->children.end() )
            return nullptr;
        node = it->get();
    }
    return node;
}

CommandRouter::Node* CommandRouter::addMenu(const QString& parentPath, const QString& name, const QString& description)
{
    Node* parent = resolveMenu(parentPath);
    if( !parent )
        parent = m_root.get();

    auto node = std::make_shared<Node>();
    node->name = name;
    node->description = description;
    node->parent = parent;
    parent->children.insert(name, node);
    return node.get();
}

void CommandRouter::addCommand(
    const QString& menuPath,
    const QString& name,
    const QString& description,
    const QStringList& paramHints,
    std::function<void(const QStringList&)> handler)
{
    Node* parent = resolveMenu(menuPath);
    if( !parent )
        parent = m_root.get();

    auto node = std::make_shared<Node>();
    node->name = name;
    node->description = description;
    node->paramHints = paramHints;
    node->handler = std::move(handler);
    node->parent = parent;
    parent->children.insert(name, node);
}

// ===========================================================================
// Tokeniser
// ===========================================================================

QStringList CommandRouter::tokenize(const QString& line)
{
    QStringList tokens;
    QString current;
    bool inDouble = false;
    bool inSingle = false;
    bool escaped = false;

    for( int i = 0; i < line.size(); ++i ) {
        QChar ch = line[i];

        if( escaped ) {
            current.append(ch);
            escaped = false;
            continue;
        }
        if( ch == QLatin1Char('\\') && !inSingle ) {
            escaped = true;
            continue;
        }
        if( ch == QLatin1Char('"') && !inSingle ) {
            inDouble = !inDouble;
            continue;
        }
        if( ch == QLatin1Char('\'') && !inDouble ) {
            inSingle = !inSingle;
            continue;
        }
        if( ch.isSpace() && !inDouble && !inSingle ) {
            if( !current.isEmpty() ) {
                tokens.append(current);
                current.clear();
            }
            continue;
        }
        current.append(ch);
    }
    if( !current.isEmpty() )
        tokens.append(current);

    return tokens;
}

// ===========================================================================
// Dispatch
// ===========================================================================

CommandRouter::Result CommandRouter::dispatch(const QString& line)
{
    QStringList tokens = tokenize(line);
    if( tokens.isEmpty() )
        return Result::Empty;

    const QString& first = tokens.first();

    // Built-in navigation
    if( first == QLatin1String("..") || (first == QLatin1String("exit") && m_current != m_root.get()) ) {
        if( navigateUp() )
            return Result::NavigatedUp;
        return Result::Exit;
    }
    if( first == QLatin1String("exit") || first == QLatin1String("quit") )
        return Result::Exit;
    if( first == QLatin1String("help") ) {
        // Help is handled by the caller (ConsoleUI) which has the terminal
        return Result::NotFound;
    }

    // Resolve depth-first through the tree
    Node* node = m_current;
    int consumed = 0;

    for( int i = 0; i < tokens.size(); ++i ) {
        auto it = node->children.find(tokens[i]);
        if( it == node->children.end() )
            break;

        Node* child = it->get();
        consumed = i + 1;

        if( !child->isMenu() ) {
            // Leaf command — invoke with remaining tokens
            QStringList args = tokens.mid(consumed);
            child->handler(args);
            return Result::Executed;
        }

        // Menu node — continue resolving
        node = child;
    }

    // If we consumed at least one token and landed on a menu, navigate
    if( consumed > 0 && node != m_current && node->isMenu() ) {
        m_current = node;
        return Result::NavigatedMenu;
    }

    return Result::NotFound;
}

// ===========================================================================
// Navigation
// ===========================================================================

QString CommandRouter::currentPath() const
{
    QStringList parts;
    const Node* n = m_current;
    while( n ) {
        parts.prepend(n->name);
        n = n->parent;
    }
    return parts.join(QLatin1Char('/'));
}

bool CommandRouter::navigateUp()
{
    if( m_current == m_root.get() )
        return false;
    m_current = m_current->parent;
    return true;
}

// ===========================================================================
// Completion
// ===========================================================================

QStringList CommandRouter::completions(const QString& partial) const
{
    QStringList result;
    for( auto it = m_current->children.cbegin(); it != m_current->children.cend(); ++it ) {
        if( it.key().startsWith(partial, Qt::CaseInsensitive) )
            result.append(it.key());
    }
    // Also add built-ins
    for( const QString& builtin : {QStringLiteral("help"), QStringLiteral("exit"), QStringLiteral("..")} ) {
        if( builtin.startsWith(partial, Qt::CaseInsensitive) )
            result.append(builtin);
    }
    result.sort();
    return result;
}

// ===========================================================================
// Help
// ===========================================================================

void CommandRouter::printHelp(Terminal& term) const
{
    term.writeLine();

    // Gather max name width for alignment
    int maxName = 0;
    for( auto it = m_current->children.cbegin(); it != m_current->children.cend(); ++it ) {
        int len = it.key().size();
        if( !it.value()->paramHints.isEmpty() ) {
            for( const QString& p : it.value()->paramHints )
                len += 1 + p.size() + 2; // " <param>"
        }
        if( len > maxName )
            maxName = len;
    }
    maxName = qMax(maxName, 6); // at least "help  "

    auto pad = [](const QString& s, int w) { return s + QString(qMax(1, w - s.size()), QLatin1Char(' ')); };

    term.writeLine(QStringLiteral("  %1%2Commands:%3").arg(Style::bold(), Style::highlight(), Style::reset()));
    term.writeLine();

    for( auto it = m_current->children.cbegin(); it != m_current->children.cend(); ++it ) {
        const Node& child = *it.value();
        QString label = child.name;
        if( !child.paramHints.isEmpty() ) {
            for( const QString& p : child.paramHints )
                label += QStringLiteral(" <%1>").arg(p);
        }
        QString tag = child.isMenu() ? QStringLiteral(" %1[menu]%2").arg(Style::muted(), Style::reset()) : QString();

        term.writeLine(
            QStringLiteral("    %1%2%3  %4%5%6")
                .arg(
                    Style::accent(),
                    pad(label, maxName + 2),
                    Style::reset(),
                    Style::dim(),
                    child.description,
                    Style::reset())
            + tag);
    }

    term.writeLine();

    // Built-in commands
    term.writeLine(QStringLiteral("    %1%2%3  %4%5%6")
                       .arg(
                           Style::accent(),
                           pad(QStringLiteral("help"), maxName + 2),
                           Style::reset(),
                           Style::dim(),
                           QStringLiteral("Show this help text"),
                           Style::reset()));
    if( m_current != m_root.get() ) {
        term.writeLine(QStringLiteral("    %1%2%3  %4%5%6")
                           .arg(
                               Style::accent(),
                               pad(QStringLiteral(".."), maxName + 2),
                               Style::reset(),
                               Style::dim(),
                               QStringLiteral("Go back to parent menu"),
                               Style::reset()));
    }
    term.writeLine(QStringLiteral("    %1%2%3  %4%5%6")
                       .arg(
                           Style::accent(),
                           pad(QStringLiteral("exit"), maxName + 2),
                           Style::reset(),
                           Style::dim(),
                           QStringLiteral("Exit aSocial"),
                           Style::reset()));
    term.writeLine();
}
