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

#ifndef CONSOLE_COMMAND_ROUTER_H
#define CONSOLE_COMMAND_ROUTER_H

#include <functional>
#include <memory>
#include <QMap>
#include <QString>
#include <QStringList>

class Terminal;

/**
 * @brief Hierarchical command router with tokenisation and tab completion.
 *
 * Commands are organised as a tree of menus (inner nodes) and leaf
 * commands.  The user can:
 *
 * - **Navigate into a submenu** by typing its name (e.g. "profile").
 * - **Execute a command from any depth** in one line
 *   (e.g. "profile create") — the router resolves through the tree.
 * - **Go back** with ".." or the submenu's "exit" alias.
 * - **Tab-complete** the names of children in the current menu.
 *
 * Command handlers receive the remaining tokens (after the command name)
 * as a QStringList so they can either use pre-supplied arguments or
 * interactively prompt for missing ones.
 */
class CommandRouter
{
public:
    /** @brief Result of dispatching a single input line. */
    enum class Result {
        Executed,      ///< A leaf command was found and invoked.
        NavigatedMenu, ///< The user entered a submenu.
        NavigatedUp,   ///< The user typed ".." or exited a submenu.
        Exit,          ///< The user typed "exit" at the root.
        NotFound,      ///< No matching command or menu.
        Empty          ///< Blank input — nothing to do.
    };

    /** @brief A single node in the command tree. */
    struct Node
    {
        QString name;
        QString description;
        QStringList paramHints;
        std::function<void(const QStringList&)> handler; ///< null for menu nodes.
        QMap<QString, std::shared_ptr<Node>> children;
        Node* parent = nullptr;

        /** @brief true when this node is a menu (has children, no handler). */
        bool isMenu() const { return handler == nullptr; }
    };

    CommandRouter();

    // --- Tree construction -------------------------------------------------

    /**
     * @brief Add a submenu to the tree.
     *
     * @param parentPath   Slash-separated path to the parent menu
     *                     (empty = root).
     * @param name         Menu name (single token, no spaces).
     * @param description  Help text for the menu.
     * @return Pointer to the created node (never null).
     */
    Node* addMenu(const QString& parentPath, const QString& name, const QString& description);

    /**
     * @brief Add a leaf command to a menu.
     *
     * @param menuPath     Slash-separated path to the parent menu.
     * @param name         Command name.
     * @param description  Help text.
     * @param paramHints   Human-readable names for the expected arguments.
     * @param handler      Callable invoked with the argument tokens.
     */
    void addCommand(
        const QString& menuPath,
        const QString& name,
        const QString& description,
        const QStringList& paramHints,
        std::function<void(const QStringList&)> handler);

    // --- Dispatch ----------------------------------------------------------

    /**
     * @brief Parse and execute a raw input line.
     *
     * The line is tokenised (respecting quoted strings and backslash
     * escapes) and resolved against the current menu.  If the first
     * token matches a submenu name, resolution continues inside that
     * submenu; if it matches a leaf command, the command's handler is
     * invoked with the remaining tokens.
     *
     * @param line  Raw text entered by the user.
     * @return Outcome of the dispatch.
     */
    Result dispatch(const QString& line);

    // --- Navigation --------------------------------------------------------

    /** @brief Slash-separated path of the current menu (e.g. "main/profile"). */
    QString currentPath() const;

    /** @brief Pointer to the current menu node. */
    Node* currentMenu() const { return m_current; }

    /** @brief Move up one level; returns false if already at the root. */
    bool navigateUp();

    // --- Completion & help -------------------------------------------------

    /**
     * @brief Return tab-completion candidates for @p partial.
     *
     * Only child names of the current menu are considered.
     */
    QStringList completions(const QString& partial) const;

    /**
     * @brief Print formatted help for the current menu to the terminal.
     * @param term  Terminal to write to.
     */
    void printHelp(Terminal& term) const;

    // --- Tokeniser (public for testability) --------------------------------

    /**
     * @brief Split an input line into tokens.
     *
     * Handles double quotes, single quotes, and backslash escaping.
     */
    static QStringList tokenize(const QString& line);

private:
    Node* resolveMenu(const QString& path);

    std::shared_ptr<Node> m_root;
    Node* m_current = nullptr;
};

#endif // CONSOLE_COMMAND_ROUTER_H
