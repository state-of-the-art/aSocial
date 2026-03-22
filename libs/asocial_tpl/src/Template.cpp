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

#include "asocial_tpl/Template.h"

#include <QRegularExpression>
#include <QStringView>

#include <functional>
#include <variant>
#include <vector>

namespace asocial_tpl {

// ================================================================
// Chunk: raw piece after splitting on delimiters
// ================================================================

enum class ChunkType { Text, Expression, Tag, Comment };

struct Chunk
{
    ChunkType type;
    QString content;
    int line;
};

static QList<Chunk> tokenize(const QString& src)
{
    QList<Chunk> chunks;
    int pos = 0;
    int line = 1;

    auto countLines = [](const QString& s, int from, int to) {
        int n = 0;
        for( int i = from; i < to; ++i )
            if( s[i] == QLatin1Char('\n') )
                ++n;
        return n;
    };

    struct Delim
    {
        QString open, close;
        ChunkType type;
    };
    const Delim delims[] = {
        {QStringLiteral("{#"), QStringLiteral("#}"), ChunkType::Comment},
        {QStringLiteral("{{"), QStringLiteral("}}"), ChunkType::Expression},
        {QStringLiteral("{%"), QStringLiteral("%}"), ChunkType::Tag},
    };

    while( pos < src.size() ) {
        int nearestPos = src.size();
        const Delim* nearestDelim = nullptr;

        for( const auto& d : delims ) {
            int idx = src.indexOf(d.open, pos);
            if( idx >= 0 && idx < nearestPos ) {
                nearestPos = idx;
                nearestDelim = &d;
            }
        }

        if( !nearestDelim ) {
            QString text = src.mid(pos);
            if( !text.isEmpty() )
                chunks.append({ChunkType::Text, text, line});
            break;
        }

        if( nearestPos > pos ) {
            chunks.append({ChunkType::Text, src.mid(pos, nearestPos - pos), line});
            line += countLines(src, pos, nearestPos);
        }

        int openEnd = nearestPos + nearestDelim->open.size();
        int closeIdx = src.indexOf(nearestDelim->close, openEnd);
        if( closeIdx < 0 ) {
            chunks.append({ChunkType::Text, src.mid(nearestPos), line});
            break;
        }

        QString content = src.mid(openEnd, closeIdx - openEnd).trimmed();
        line += countLines(src, nearestPos, closeIdx + nearestDelim->close.size());

        if( nearestDelim->type != ChunkType::Comment )
            chunks.append({nearestDelim->type, content, line});

        pos = closeIdx + nearestDelim->close.size();

        if( nearestDelim->type == ChunkType::Tag && pos < src.size() && src[pos] == QLatin1Char('\n') ) {
            ++pos;
            ++line;
        }
    }
    return chunks;
}

// ================================================================
// Expression AST
// ================================================================

struct ExprNode;
using ExprPtr = std::shared_ptr<ExprNode>;

enum class ExprType { Literal, Variable, DotAccess, Filter, BinOp, UnaryNot, FuncCall, ListLiteral };

struct ExprNode
{
    ExprType type;
    QVariant literal;
    QString name;
    ExprPtr left, right;
    QList<ExprPtr> args;
};

static ExprPtr makeLiteral(const QVariant& v)
{
    auto n = std::make_shared<ExprNode>();
    n->type = ExprType::Literal;
    n->literal = v;
    return n;
}
static ExprPtr makeVar(const QString& name)
{
    auto n = std::make_shared<ExprNode>();
    n->type = ExprType::Variable;
    n->name = name;
    return n;
}
static ExprPtr makeDot(ExprPtr obj, const QString& field)
{
    auto n = std::make_shared<ExprNode>();
    n->type = ExprType::DotAccess;
    n->left = std::move(obj);
    n->name = field;
    return n;
}
static ExprPtr makeFilter(ExprPtr input, const QString& filterName, QList<ExprPtr> args)
{
    auto n = std::make_shared<ExprNode>();
    n->type = ExprType::Filter;
    n->left = std::move(input);
    n->name = filterName;
    n->args = std::move(args);
    return n;
}
static ExprPtr makeBinOp(const QString& op, ExprPtr l, ExprPtr r)
{
    auto n = std::make_shared<ExprNode>();
    n->type = ExprType::BinOp;
    n->name = op;
    n->left = std::move(l);
    n->right = std::move(r);
    return n;
}
static ExprPtr makeUnaryNot(ExprPtr operand)
{
    auto n = std::make_shared<ExprNode>();
    n->type = ExprType::UnaryNot;
    n->left = std::move(operand);
    return n;
}

// ================================================================
// Expression Parser (recursive descent)
// ================================================================

class ExprParser
{
public:
    explicit ExprParser(const QString& src)
        : m_src(src.trimmed())
    {}

    ExprPtr parse() { return parseOr(); }
    bool atEnd() const { return m_pos >= m_src.size(); }
    int pos() const { return m_pos; }

private:
    QString m_src;
    int m_pos = 0;

    void skipWS()
    {
        while( m_pos < m_src.size() && m_src[m_pos].isSpace() )
            ++m_pos;
    }

    QChar peek()
    {
        skipWS();
        return m_pos < m_src.size() ? m_src[m_pos] : QChar();
    }

    bool matchStr(const QString& s)
    {
        skipWS();
        if( QStringView(m_src).mid(m_pos).startsWith(s) ) {
            if( s[0].isLetter() && m_pos + s.size() < m_src.size()
                && (m_src[m_pos + s.size()].isLetterOrNumber() || m_src[m_pos + s.size()] == QLatin1Char('_')) )
                return false;
            m_pos += s.size();
            return true;
        }
        return false;
    }

    QString readIdentifier()
    {
        skipWS();
        int start = m_pos;
        while( m_pos < m_src.size() && (m_src[m_pos].isLetterOrNumber() || m_src[m_pos] == QLatin1Char('_')) )
            ++m_pos;
        return m_src.mid(start, m_pos - start);
    }

    QString readString(QChar quote)
    {
        ++m_pos;
        QString result;
        while( m_pos < m_src.size() && m_src[m_pos] != quote ) {
            if( m_src[m_pos] == QLatin1Char('\\') && m_pos + 1 < m_src.size() ) {
                ++m_pos;
                if( m_src[m_pos] == QLatin1Char('n') )
                    result += QLatin1Char('\n');
                else if( m_src[m_pos] == QLatin1Char('t') )
                    result += QLatin1Char('\t');
                else
                    result += m_src[m_pos];
            } else {
                result += m_src[m_pos];
            }
            ++m_pos;
        }
        if( m_pos < m_src.size() )
            ++m_pos;
        return result;
    }

    ExprPtr parseOr()
    {
        auto left = parseAnd();
        while( matchStr(QStringLiteral("or")) )
            left = makeBinOp(QStringLiteral("or"), left, parseAnd());
        return left;
    }

    ExprPtr parseAnd()
    {
        auto left = parseNot();
        while( matchStr(QStringLiteral("and")) )
            left = makeBinOp(QStringLiteral("and"), left, parseNot());
        return left;
    }

    ExprPtr parseNot()
    {
        if( matchStr(QStringLiteral("not")) )
            return makeUnaryNot(parseNot());
        return parseComparison();
    }

    ExprPtr parseComparison()
    {
        auto left = parseAddSub();

        skipWS();
        struct CmpOp
        {
            QString text;
        };
        static const CmpOp ops[] = {
            {QStringLiteral("==")},
            {QStringLiteral("!=")},
            {QStringLiteral("<=")},
            {QStringLiteral(">=")},
            {QStringLiteral("<")},
            {QStringLiteral(">")},
        };

        for( const auto& op : ops ) {
            if( matchStr(op.text) )
                return makeBinOp(op.text, left, parseAddSub());
        }

        if( matchStr(QStringLiteral("not in")) )
            return makeUnaryNot(makeBinOp(QStringLiteral("in"), left, parseAddSub()));

        if( matchStr(QStringLiteral("in")) )
            return makeBinOp(QStringLiteral("in"), left, parseAddSub());

        return left;
    }

    ExprPtr parseAddSub()
    {
        auto left = parseMulDiv();
        while( true ) {
            skipWS();
            if( matchStr(QStringLiteral("+")) )
                left = makeBinOp(QStringLiteral("+"), left, parseMulDiv());
            else if( matchStr(QStringLiteral("-")) )
                left = makeBinOp(QStringLiteral("-"), left, parseMulDiv());
            else
                break;
        }
        return left;
    }

    ExprPtr parseMulDiv()
    {
        auto left = parseFilter();
        while( true ) {
            skipWS();
            if( matchStr(QStringLiteral("*")) )
                left = makeBinOp(QStringLiteral("*"), left, parseFilter());
            else if( matchStr(QStringLiteral("//")) )
                left = makeBinOp(QStringLiteral("//"), left, parseFilter());
            else if( matchStr(QStringLiteral("/")) )
                left = makeBinOp(QStringLiteral("/"), left, parseFilter());
            else if( matchStr(QStringLiteral("%")) )
                left = makeBinOp(QStringLiteral("%"), left, parseFilter());
            else
                break;
        }
        return left;
    }

    ExprPtr parseFilter()
    {
        auto expr = parsePrimary();
        while( peek() == QLatin1Char('|') ) {
            ++m_pos;
            QString filterName = readIdentifier();
            QList<ExprPtr> filterArgs;
            if( peek() == QLatin1Char('(') ) {
                ++m_pos;
                if( peek() != QLatin1Char(')') ) {
                    filterArgs.append(parse());
                    while( peek() == QLatin1Char(',') ) {
                        ++m_pos;
                        filterArgs.append(parse());
                    }
                }
                if( peek() == QLatin1Char(')') )
                    ++m_pos;
            }
            expr = makeFilter(expr, filterName, filterArgs);
        }
        return expr;
    }

    ExprPtr parsePrimary()
    {
        skipWS();
        if( m_pos >= m_src.size() )
            return makeLiteral(QVariant());

        QChar ch = m_src[m_pos];

        if( ch == QLatin1Char('"') || ch == QLatin1Char('\'') )
            return makeLiteral(readString(ch));

        if( ch == QLatin1Char('(') ) {
            ++m_pos;
            auto expr = parse();
            if( peek() == QLatin1Char(')') )
                ++m_pos;
            return expr;
        }

        if( ch == QLatin1Char('[') ) {
            ++m_pos;
            auto node = std::make_shared<ExprNode>();
            node->type = ExprType::ListLiteral;
            if( peek() != QLatin1Char(']') ) {
                node->args.append(parse());
                while( peek() == QLatin1Char(',') ) {
                    ++m_pos;
                    node->args.append(parse());
                }
            }
            if( peek() == QLatin1Char(']') )
                ++m_pos;
            return node;
        }

        if( ch.isDigit() || (ch == QLatin1Char('-') && m_pos + 1 < m_src.size() && m_src[m_pos + 1].isDigit()) ) {
            int start = m_pos;
            if( ch == QLatin1Char('-') )
                ++m_pos;
            bool hasDot = false;
            while( m_pos < m_src.size() && (m_src[m_pos].isDigit() || m_src[m_pos] == QLatin1Char('.')) ) {
                if( m_src[m_pos] == QLatin1Char('.') )
                    hasDot = true;
                ++m_pos;
            }
            QString num = m_src.mid(start, m_pos - start);
            return hasDot ? makeLiteral(num.toDouble()) : makeLiteral(num.toLongLong());
        }

        if( ch.isLetter() || ch == QLatin1Char('_') ) {
            int saved = m_pos;
            QString id = readIdentifier();
            if( id == QLatin1String("true") )
                return makeLiteral(true);
            if( id == QLatin1String("false") )
                return makeLiteral(false);
            if( id == QLatin1String("none") || id == QLatin1String("None") )
                return makeLiteral(QVariant());

            auto expr = makeVar(id);

            while( peek() == QLatin1Char('.') ) {
                ++m_pos;
                QString field = readIdentifier();
                if( !field.isEmpty() )
                    expr = makeDot(expr, field);
            }
            return expr;
        }

        ++m_pos;
        return makeLiteral(QVariant());
    }
};

// ================================================================
// Template AST nodes
// ================================================================

enum class NodeType { Text, Output, If, For, Set, Block };

struct TplNode;
using TplNodePtr = std::shared_ptr<TplNode>;

struct IfBranch
{
    ExprPtr condition;
    QList<TplNodePtr> body;
};

struct TplNode
{
    NodeType type;
    QString text;
    ExprPtr expr;
    QList<TplNodePtr> children;

    QList<IfBranch> branches;
    QList<TplNodePtr> elseBody;

    QString varName;
    ExprPtr iterExpr;
};

// ================================================================
// Template Parser
// ================================================================

class TplParser
{
public:
    explicit TplParser(const QList<Chunk>& chunks)
        : m_chunks(chunks)
    {}

    QList<TplNodePtr> parse() { return parseBlock({}); }
    QString error() const { return m_error; }

private:
    QList<Chunk> m_chunks;
    int m_pos = 0;
    QString m_error;

    bool atEnd() const { return m_pos >= m_chunks.size(); }
    const Chunk& current() const { return m_chunks[m_pos]; }

    QList<TplNodePtr> parseBlock(const QStringList& endTags)
    {
        QList<TplNodePtr> nodes;
        while( !atEnd() ) {
            const auto& ch = current();
            if( ch.type == ChunkType::Tag ) {
                QString keyword = ch.content.section(QLatin1Char(' '), 0, 0);
                if( endTags.contains(keyword) )
                    return nodes;
            }
            auto node = parseNode();
            if( node )
                nodes.append(node);
        }
        return nodes;
    }

    TplNodePtr parseNode()
    {
        if( atEnd() )
            return {};
        const auto& ch = current();

        if( ch.type == ChunkType::Text ) {
            auto node = std::make_shared<TplNode>();
            node->type = NodeType::Text;
            node->text = ch.content;
            ++m_pos;
            return node;
        }

        if( ch.type == ChunkType::Expression ) {
            auto node = std::make_shared<TplNode>();
            node->type = NodeType::Output;
            ExprParser ep(ch.content);
            node->expr = ep.parse();
            ++m_pos;
            return node;
        }

        if( ch.type == ChunkType::Tag ) {
            QString keyword = ch.content.section(QLatin1Char(' '), 0, 0);
            if( keyword == QLatin1String("if") )
                return parseIf();
            if( keyword == QLatin1String("for") )
                return parseFor();
            if( keyword == QLatin1String("set") )
                return parseSet();

            ++m_pos;
            return {};
        }

        ++m_pos;
        return {};
    }

    TplNodePtr parseIf()
    {
        auto node = std::make_shared<TplNode>();
        node->type = NodeType::If;

        QString condStr = current().content.mid(3).trimmed();
        ExprParser ep(condStr);
        IfBranch branch;
        branch.condition = ep.parse();
        ++m_pos;

        branch.body = parseBlock({QStringLiteral("elif"), QStringLiteral("else"), QStringLiteral("endif")});
        node->branches.append(branch);

        while( !atEnd() && current().type == ChunkType::Tag ) {
            QString kw = current().content.section(QLatin1Char(' '), 0, 0);
            if( kw == QLatin1String("elif") ) {
                QString elifCond = current().content.mid(5).trimmed();
                ExprParser ep2(elifCond);
                IfBranch elifBranch;
                elifBranch.condition = ep2.parse();
                ++m_pos;
                elifBranch.body = parseBlock({QStringLiteral("elif"), QStringLiteral("else"), QStringLiteral("endif")});
                node->branches.append(elifBranch);
            } else if( kw == QLatin1String("else") ) {
                ++m_pos;
                node->elseBody = parseBlock({QStringLiteral("endif")});
            } else if( kw == QLatin1String("endif") ) {
                ++m_pos;
                break;
            } else {
                break;
            }
        }
        return node;
    }

    TplNodePtr parseFor()
    {
        auto node = std::make_shared<TplNode>();
        node->type = NodeType::For;

        QString rest = current().content.mid(4).trimmed();
        int inIdx = rest.indexOf(QStringLiteral(" in "));
        if( inIdx < 0 ) {
            m_error = QStringLiteral("Expected 'in' in for statement");
            ++m_pos;
            return node;
        }
        node->varName = rest.left(inIdx).trimmed();
        QString iterStr = rest.mid(inIdx + 4).trimmed();
        ExprParser ep(iterStr);
        node->iterExpr = ep.parse();
        ++m_pos;

        node->children = parseBlock({QStringLiteral("endfor")});

        if( !atEnd() && current().type == ChunkType::Tag && current().content.trimmed() == QLatin1String("endfor") )
            ++m_pos;

        return node;
    }

    TplNodePtr parseSet()
    {
        auto node = std::make_shared<TplNode>();
        node->type = NodeType::Set;

        QString rest = current().content.mid(4).trimmed();
        int eqIdx = rest.indexOf(QLatin1Char('='));
        if( eqIdx < 0 ) {
            ++m_pos;
            return node;
        }
        node->varName = rest.left(eqIdx).trimmed();
        QString valStr = rest.mid(eqIdx + 1).trimmed();
        ExprParser ep(valStr);
        node->expr = ep.parse();
        ++m_pos;
        return node;
    }
};

// ================================================================
// Renderer
// ================================================================

class Renderer
{
public:
    QString render(const QList<TplNodePtr>& nodes, const QVariantMap& context)
    {
        m_context = context;
        QString result;
        renderBlock(nodes, result);
        return result;
    }

private:
    QVariantMap m_context;

    void renderBlock(const QList<TplNodePtr>& nodes, QString& out)
    {
        for( const auto& node : nodes )
            renderNode(node, out);
    }

    void renderNode(const TplNodePtr& node, QString& out)
    {
        if( !node )
            return;

        switch( node->type ) {
        case NodeType::Text:
            out += node->text;
            break;
        case NodeType::Output:
            out += variantToString(evalExpr(node->expr));
            break;
        case NodeType::If:
            renderIf(node, out);
            break;
        case NodeType::For:
            renderFor(node, out);
            break;
        case NodeType::Set:
            m_context[node->varName] = evalExpr(node->expr);
            break;
        default:
            break;
        }
    }

    void renderIf(const TplNodePtr& node, QString& out)
    {
        for( const auto& branch : node->branches ) {
            if( isTruthy(evalExpr(branch.condition)) ) {
                renderBlock(branch.body, out);
                return;
            }
        }
        renderBlock(node->elseBody, out);
    }

    void renderFor(const TplNodePtr& node, QString& out)
    {
        QVariant iterVal = evalExpr(node->iterExpr);
        QVariantList list;
        if( iterVal.typeId() == QMetaType::QVariantList )
            list = iterVal.toList();
        else if( iterVal.typeId() == QMetaType::QStringList ) {
            const auto sl = iterVal.toStringList();
            for( const auto& s : sl )
                list.append(s);
        }

        QVariant savedVar = m_context.value(node->varName);
        QVariant savedLoop = m_context.value(QStringLiteral("loop"));

        for( int i = 0; i < list.size(); ++i ) {
            m_context[node->varName] = list[i];

            QVariantMap loopVar;
            loopVar[QStringLiteral("index")] = i + 1;
            loopVar[QStringLiteral("index0")] = i;
            loopVar[QStringLiteral("first")] = (i == 0);
            loopVar[QStringLiteral("last")] = (i == list.size() - 1);
            loopVar[QStringLiteral("length")] = list.size();
            m_context[QStringLiteral("loop")] = loopVar;

            renderBlock(node->children, out);
        }

        if( savedVar.isValid() )
            m_context[node->varName] = savedVar;
        else
            m_context.remove(node->varName);

        if( savedLoop.isValid() )
            m_context[QStringLiteral("loop")] = savedLoop;
        else
            m_context.remove(QStringLiteral("loop"));
    }

    QVariant evalExpr(const ExprPtr& expr)
    {
        if( !expr )
            return QVariant();

        switch( expr->type ) {
        case ExprType::Literal:
            return expr->literal;

        case ExprType::Variable:
            return m_context.value(expr->name);

        case ExprType::DotAccess: {
            QVariant obj = evalExpr(expr->left);
            if( obj.typeId() == QMetaType::QVariantMap )
                return obj.toMap().value(expr->name);
            return QVariant();
        }

        case ExprType::Filter:
            return applyFilter(evalExpr(expr->left), expr->name, expr->args);

        case ExprType::BinOp:
            return evalBinOp(expr->name, expr->left, expr->right);

        case ExprType::UnaryNot:
            return !isTruthy(evalExpr(expr->left));

        case ExprType::ListLiteral: {
            QVariantList result;
            for( const auto& a : expr->args )
                result.append(evalExpr(a));
            return result;
        }

        default:
            return QVariant();
        }
    }

    QVariant evalBinOp(const QString& op, const ExprPtr& left, const ExprPtr& right)
    {
        if( op == QLatin1String("and") )
            return isTruthy(evalExpr(left)) && isTruthy(evalExpr(right));
        if( op == QLatin1String("or") )
            return isTruthy(evalExpr(left)) || isTruthy(evalExpr(right));

        QVariant lv = evalExpr(left);
        QVariant rv = evalExpr(right);

        if( op == QLatin1String("in") ) {
            if( rv.typeId() == QMetaType::QVariantList ) {
                return rv.toList().contains(lv);
            }
            if( rv.typeId() == QMetaType::QStringList ) {
                return rv.toStringList().contains(lv.toString());
            }
            if( rv.typeId() == QMetaType::QString ) {
                return rv.toString().contains(lv.toString());
            }
            return false;
        }

        if( op == QLatin1String("+") ) {
            if( lv.typeId() == QMetaType::QString || rv.typeId() == QMetaType::QString )
                return lv.toString() + rv.toString();
            return lv.toDouble() + rv.toDouble();
        }
        if( op == QLatin1String("-") )
            return lv.toDouble() - rv.toDouble();
        if( op == QLatin1String("*") )
            return lv.toDouble() * rv.toDouble();
        if( op == QLatin1String("/") )
            return rv.toDouble() != 0.0 ? lv.toDouble() / rv.toDouble() : 0.0;
        if( op == QLatin1String("//") )
            return rv.toLongLong() != 0 ? lv.toLongLong() / rv.toLongLong() : 0LL;
        if( op == QLatin1String("%") )
            return rv.toLongLong() != 0 ? lv.toLongLong() % rv.toLongLong() : 0LL;

        if( op == QLatin1String("==") )
            return lv == rv || lv.toString() == rv.toString();
        if( op == QLatin1String("!=") )
            return !(lv == rv || lv.toString() == rv.toString());
        if( op == QLatin1String("<") )
            return lv.toDouble() < rv.toDouble();
        if( op == QLatin1String(">") )
            return lv.toDouble() > rv.toDouble();
        if( op == QLatin1String("<=") )
            return lv.toDouble() <= rv.toDouble();
        if( op == QLatin1String(">=") )
            return lv.toDouble() >= rv.toDouble();

        return QVariant();
    }

    QVariant applyFilter(const QVariant& input, const QString& name, const QList<ExprPtr>& args)
    {
        QString s = variantToString(input);

        auto argVal = [&](int i) -> QVariant { return i < args.size() ? evalExpr(args[i]) : QVariant(); };

        if( name == QLatin1String("upper") )
            return s.toUpper();
        if( name == QLatin1String("lower") )
            return s.toLower();
        if( name == QLatin1String("capitalize") )
            return s.isEmpty() ? s : s[0].toUpper() + s.mid(1);
        if( name == QLatin1String("title") ) {
            QStringList words = s.split(QLatin1Char(' '));
            for( auto& w : words )
                if( !w.isEmpty() )
                    w[0] = w[0].toUpper();
            return words.join(QLatin1Char(' '));
        }
        if( name == QLatin1String("trim") )
            return s.trimmed();
        if( name == QLatin1String("length") || name == QLatin1String("count") ) {
            if( input.typeId() == QMetaType::QVariantList )
                return input.toList().size();
            if( input.typeId() == QMetaType::QStringList )
                return input.toStringList().size();
            return s.size();
        }
        if( name == QLatin1String("default") ) {
            if( !input.isValid() || s.isEmpty() )
                return argVal(0);
            return input;
        }
        if( name == QLatin1String("join") ) {
            QString sep = argVal(0).toString();
            if( input.typeId() == QMetaType::QVariantList ) {
                QStringList parts;
                for( const auto& v : input.toList() )
                    parts << variantToString(v);
                return parts.join(sep);
            }
            if( input.typeId() == QMetaType::QStringList )
                return input.toStringList().join(sep);
            return s;
        }
        if( name == QLatin1String("replace") )
            return s.replace(argVal(0).toString(), argVal(1).toString());
        if( name == QLatin1String("first") ) {
            if( input.typeId() == QMetaType::QVariantList ) {
                auto l = input.toList();
                return l.isEmpty() ? QVariant() : l.first();
            }
            return s.isEmpty() ? QString() : QString(s[0]);
        }
        if( name == QLatin1String("last") ) {
            if( input.typeId() == QMetaType::QVariantList ) {
                auto l = input.toList();
                return l.isEmpty() ? QVariant() : l.last();
            }
            return s.isEmpty() ? QString() : QString(s[s.size() - 1]);
        }
        if( name == QLatin1String("sort") ) {
            if( input.typeId() == QMetaType::QVariantList ) {
                auto l = input.toList();
                std::sort(l.begin(), l.end(), [](const QVariant& a, const QVariant& b) {
                    return a.toString() < b.toString();
                });
                return l;
            }
            return input;
        }
        if( name == QLatin1String("reverse") ) {
            if( input.typeId() == QMetaType::QVariantList ) {
                auto l = input.toList();
                std::reverse(l.begin(), l.end());
                return l;
            }
            return input;
        }

        if( name == QLatin1String("snake_case") ) {
            QString result;
            for( int i = 0; i < s.size(); ++i ) {
                QChar c = s[i];
                if( c.isUpper() ) {
                    if( i > 0 )
                        result += QLatin1Char('_');
                    result += c.toLower();
                } else {
                    result += c;
                }
            }
            return result;
        }
        if( name == QLatin1String("camel_case") ) {
            QStringList parts = s.split(QLatin1Char('_'), Qt::SkipEmptyParts);
            if( parts.isEmpty() )
                return s;
            QString result = parts[0].toLower();
            for( int i = 1; i < parts.size(); ++i ) {
                if( !parts[i].isEmpty() )
                    result += parts[i][0].toUpper() + parts[i].mid(1).toLower();
            }
            return result;
        }
        if( name == QLatin1String("pascal_case") ) {
            QStringList parts = s.split(QLatin1Char('_'), Qt::SkipEmptyParts);
            QString result;
            for( const auto& p : parts )
                if( !p.isEmpty() )
                    result += p[0].toUpper() + p.mid(1).toLower();
            return result;
        }

        return input;
    }

    static bool isTruthy(const QVariant& v)
    {
        if( !v.isValid() )
            return false;
        if( v.typeId() == QMetaType::Bool )
            return v.toBool();
        if( v.typeId() == QMetaType::Int || v.typeId() == QMetaType::LongLong )
            return v.toLongLong() != 0;
        if( v.typeId() == QMetaType::Double )
            return v.toDouble() != 0.0;
        if( v.typeId() == QMetaType::QString )
            return !v.toString().isEmpty();
        if( v.typeId() == QMetaType::QVariantList )
            return !v.toList().isEmpty();
        if( v.typeId() == QMetaType::QVariantMap )
            return !v.toMap().isEmpty();
        return true;
    }

    static QString variantToString(const QVariant& v)
    {
        if( !v.isValid() )
            return {};
        if( v.typeId() == QMetaType::Bool )
            return v.toBool() ? QStringLiteral("true") : QStringLiteral("false");
        if( v.typeId() == QMetaType::QVariantList ) {
            QStringList parts;
            for( const auto& item : v.toList() )
                parts << variantToString(item);
            return QLatin1Char('[') + parts.join(QStringLiteral(", ")) + QLatin1Char(']');
        }
        return v.toString();
    }
};

// ================================================================
// Template::Impl
// ================================================================

struct Template::Impl
{
    QList<TplNodePtr> nodes;
    QString error;
};

Template::Template(const QString& source)
    : m_impl(std::make_unique<Impl>())
{
    auto chunks = tokenize(source);
    TplParser parser(chunks);
    m_impl->nodes = parser.parse();
    m_impl->error = parser.error();
}

Template::~Template() = default;
Template::Template(Template&&) noexcept = default;
Template& Template::operator=(Template&&) noexcept = default;

QString Template::render(const QVariantMap& context) const
{
    Renderer renderer;
    return renderer.render(m_impl->nodes, context);
}

QString Template::renderString(const QString& source, const QVariantMap& context)
{
    Template tpl(source);
    return tpl.render(context);
}

bool Template::hasError() const
{
    return !m_impl->error.isEmpty();
}
QString Template::errorMessage() const
{
    return m_impl->error;
}

} // namespace asocial_tpl
