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

#ifndef ASOCIAL_TPL_TEMPLATE_H
#define ASOCIAL_TPL_TEMPLATE_H

#include <memory>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

namespace asocial_tpl {

/**
 * @brief Jinja2-like template engine using Qt6 Core types.
 *
 * Supported syntax:
 *   {{ expr }}                       - Output an expression
 *   {% if cond %} ... {% endif %}    - Conditional (elif/else supported)
 *   {% for x in list %} ... {% endfor %} - Iteration
 *   {% set x = expr %}               - Variable assignment
 *   {# comment #}                    - Comment (ignored)
 *
 * Expressions:
 *   Variable access:    name, obj.field
 *   Filters:            name|upper, name|lower, name|join(",")
 *   Literals:           "string", 42, true, false
 *   Comparisons:        ==, !=, <, >, <=, >=
 *   Logic:              and, or, not
 *   Membership:         x in list
 *
 * Filters: upper, lower, capitalize, title, trim, length, default(val),
 *          join(sep), replace(old,new), snake_case, camel_case,
 *          pascal_case, first, last, sort, reverse, count
 *
 * Loop variables: loop.index (1-based), loop.index0 (0-based),
 *                 loop.first, loop.last, loop.length
 *
 * Context values use QVariant: QString, int, double, bool,
 *                               QVariantMap (dot access), QVariantList (for loops)
 */
class Template
{
public:
    explicit Template(const QString& source);
    ~Template();

    Template(Template&&) noexcept;
    Template& operator=(Template&&) noexcept;

    QString render(const QVariantMap& context) const;

    static QString renderString(const QString& source, const QVariantMap& context);

    bool hasError() const;
    QString errorMessage() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace asocial_tpl

#endif // ASOCIAL_TPL_TEMPLATE_H
