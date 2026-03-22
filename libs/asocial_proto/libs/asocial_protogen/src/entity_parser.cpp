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

#include "asocial_protogen/entity_parser.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>

#include <QRegularExpression>
#include <QVariantList>
#include <QVariantMap>

namespace pb = google::protobuf;

namespace asocial_protogen {

namespace {

QString toSnakeCase(const QString& name)
{
    QString result;
    for( int i = 0; i < name.size(); ++i ) {
        QChar c = name[i];
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

QString toUpperCase(const QString& name)
{
    return toSnakeCase(name).toUpper();
}

QString toCamelCase(const QString& snake)
{
    QStringList parts = snake.split(QLatin1Char('_'), Qt::SkipEmptyParts);
    if( parts.isEmpty() )
        return snake;
    QString result = parts[0];
    for( int i = 1; i < parts.size(); ++i )
        if( !parts[i].isEmpty() )
            result += parts[i][0].toUpper() + parts[i].mid(1);
    return result;
}

QString toPascalCase(const QString& snake)
{
    QStringList parts = snake.split(QLatin1Char('_'), Qt::SkipEmptyParts);
    QString result;
    for( const auto& p : parts )
        if( !p.isEmpty() )
            result += p[0].toUpper() + p.mid(1);
    return result;
}

QString pluralize(const QString& name)
{
    if( name.endsWith(QLatin1String("y")) && !name.endsWith(QLatin1String("ey")) )
        return name.left(name.size() - 1) + QLatin1String("ies");
    if( name.endsWith(QLatin1String("s")) || name.endsWith(QLatin1String("x")) || name.endsWith(QLatin1String("sh"))
        || name.endsWith(QLatin1String("ch")) )
        return name + QLatin1String("es");
    return name + QLatin1Char('s');
}

QString getEntityKeyPrefix(const std::string& entityName, const std::map<std::string, EntityInfo>& entityMap)
{
    auto it = entityMap.find(entityName);
    if( it != entityMap.end() )
        return it->second.keyPrefix;
    return {};
}

QList<FKTarget> parseFKAnnotation(const std::string& fkStr, const std::map<std::string, EntityInfo>& entityMap)
{
    QList<FKTarget> targets;
    QString fk = QString::fromStdString(fkStr);
    const auto parts = fk.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for( const auto& part : parts ) {
        auto trimmed = part.trimmed();
        int dotIdx = trimmed.indexOf(QLatin1Char('.'));
        if( dotIdx < 0 )
            continue;
        FKTarget t;
        t.entity = trimmed.left(dotIdx);
        t.field = trimmed.mid(dotIdx + 1);
        t.keyPrefix = getEntityKeyPrefix(t.entity.toStdString(), entityMap);
        targets.append(t);
    }
    return targets;
}

struct EntityConfigData
{
    std::string key_prefix;
    std::string key_pattern;
    bool singleton = false;
    bool persona_scoped = false;
    std::string sort_field;
    bool sort_descending = false;
};

struct DepsConfigData
{
    std::string fk;
};

bool getEntityConfig(const pb::Descriptor* msg, EntityConfigData& out)
{
    const auto& opts = msg->options();
    const auto* pool = msg->file()->pool();
    const auto* extDesc = pool->FindExtensionByName("asocial.v1.entity_config");
    if( !extDesc )
        return false;

    pb::DynamicMessageFactory factory(pool);
    const pb::Message* optProto = factory.GetPrototype(extDesc->containing_type());
    std::unique_ptr<pb::Message> optsCopy(optProto->New());
    std::string wire;
    opts.SerializeToString(&wire);
    if( !optsCopy->ParseFromString(wire) )
        return false;
    const auto& reflection = *optsCopy->GetReflection();
    if( !reflection.HasField(*optsCopy, extDesc) )
        return false;

    const auto& submsg = reflection.GetMessage(*optsCopy, extDesc);
    const auto* cfgDesc = submsg.GetDescriptor();

    auto getStr = [&](const char* name) -> std::string {
        auto* fd = cfgDesc->FindFieldByName(name);
        return fd ? submsg.GetReflection()->GetString(submsg, fd) : "";
    };
    auto getBool = [&](const char* name) -> bool {
        auto* fd = cfgDesc->FindFieldByName(name);
        return fd ? submsg.GetReflection()->GetBool(submsg, fd) : false;
    };

    out.key_prefix = getStr("key_prefix");
    out.key_pattern = getStr("key_pattern");
    out.singleton = getBool("singleton");
    out.persona_scoped = getBool("persona_scoped");
    out.sort_field = getStr("sort_field");
    out.sort_descending = getBool("sort_descending");
    return true;
}

bool getDepsConfig(const pb::FieldDescriptor* field, DepsConfigData& out)
{
    const auto& opts = field->options();
    const auto* pool = field->file()->pool();
    const auto* extDesc = pool->FindExtensionByName("asocial.v1.deps_config");
    if( !extDesc )
        return false;

    pb::DynamicMessageFactory factory(pool);
    const pb::Message* optProto = factory.GetPrototype(extDesc->containing_type());
    std::unique_ptr<pb::Message> optsCopy(optProto->New());
    std::string wire;
    opts.SerializeToString(&wire);
    if( !optsCopy->ParseFromString(wire) )
        return false;
    const auto& reflection = *optsCopy->GetReflection();
    if( !reflection.HasField(*optsCopy, extDesc) )
        return false;

    const auto& submsg = reflection.GetMessage(*optsCopy, extDesc);
    const auto* cfgDesc = submsg.GetDescriptor();
    auto* fkField = cfgDesc->FindFieldByName("fk");
    if( fkField )
        out.fk = submsg.GetReflection()->GetString(submsg, fkField);
    return true;
}

// Timestamp in keys: ISO 8601 "yyyy-MM-ddTHH:mm:ss" in UTC for lexicographic filtering
static const char* TIMESTAMP_PARAM_TYPE = "const QString&";
static const char* TIMESTAMP_PARAM_SUFFIX = "Iso";

void buildKeyFunctions(EntityInfo& e)
{
    if( e.singleton ) {
        e.constName = e.nameUpper;
        e.fnName = e.nameSnake;
        return;
    }

    e.constName = e.nameUpper;
    e.fnName = e.nameSnake;

    QStringList segments;
    static QRegularExpression re(QStringLiteral("\\{([^}]+)\\}"));
    auto it = re.globalMatch(e.keyPattern);
    while( it.hasNext() ) {
        auto match = it.next();
        segments.append(match.captured(1));
    }

    QList<KeySegment> keySegments;
    for( const auto& seg : segments ) {
        KeySegment ks;
        ks.name = seg;
        ks.paramName = toCamelCase(seg);
        for( const auto& f : e.allFields ) {
            if( f.protoName == seg && f.isTimestamp ) {
                ks.isTimestamp = true;
                break;
            }
        }
        keySegments.append(ks);
    }

    // Full key function
    QStringList params;
    QStringList bodyParts;
    for( const auto& ks : keySegments ) {
        if( ks.isTimestamp ) {
            params.append(
                QString::fromUtf8(TIMESTAMP_PARAM_TYPE) + QLatin1Char(' ') + ks.paramName
                + QString::fromUtf8(TIMESTAMP_PARAM_SUFFIX));
            bodyParts.append(ks.paramName + QString::fromUtf8(TIMESTAMP_PARAM_SUFFIX));
        } else {
            params.append(QStringLiteral("const QString& ") + ks.paramName);
            bodyParts.append(ks.paramName);
        }
    }
    e.keyFnParams = params.join(QStringLiteral(", "));
    e.keyFnBody = bodyParts.join(QStringLiteral(" + '/' + "));

    // Prefix overloads: 0 params = type prefix only, 1..n = gradually add segments
    e.prefixOverloads.clear();
    for( int i = 0; i <= keySegments.size(); ++i ) {
        PrefixOverload po;
        po.paramCount = i;
        if( i == 0 ) {
            po.params = QString();
            po.body = QStringLiteral("return ") + e.constName + QStringLiteral("_PFX");
        } else {
            QStringList poParams;
            QStringList poParts;
            for( int j = 0; j < i; ++j ) {
                const auto& ks = keySegments[j];
                if( ks.isTimestamp ) {
                    poParams.append(
                        QString::fromUtf8(TIMESTAMP_PARAM_TYPE) + QLatin1Char(' ') + ks.paramName
                        + QString::fromUtf8(TIMESTAMP_PARAM_SUFFIX));
                } else {
                    poParams.append(QStringLiteral("const QString& ") + ks.paramName);
                }
                poParts.append(
                    ks.isTimestamp ? (ks.paramName + QString::fromUtf8(TIMESTAMP_PARAM_SUFFIX)) : ks.paramName);
            }
            po.params = poParams.join(QStringLiteral(", "));
            po.body = QStringLiteral("return ") + e.constName + QStringLiteral("_PFX + ")
                      + poParts.join(QStringLiteral(" + '/' + ")) + QStringLiteral(" + '/'");
        }
        e.prefixOverloads.append(po);
    }
}

} // namespace

std::vector<EntityInfo> buildEntities(const std::vector<const pb::FileDescriptor*>& files)
{
    std::map<std::string, EntityInfo> entityMap;

    for( const auto* file : files ) {
        for( int m = 0; m < file->message_type_count(); ++m ) {
            const auto* msg = file->message_type(m);
            EntityConfigData ec;
            if( !getEntityConfig(msg, ec) )
                continue;

            EntityInfo e;
            e.protoSourceFile = QString::fromStdString(file->name());
            e.name = QString::fromStdString(msg->name());
            e.nameSnake = toSnakeCase(e.name);
            e.nameUpper = toUpperCase(e.name);
            e.namePlural = pluralize(e.name);
            e.cppType = QStringLiteral("asocial::v1::") + e.name;
            e.keyPrefix = QString::fromStdString(ec.key_prefix);
            e.keyPattern = QString::fromStdString(ec.key_pattern);
            e.singleton = ec.singleton;
            e.personaScoped = ec.persona_scoped;
            e.sortField = QString::fromStdString(ec.sort_field);
            e.sortDescending = ec.sort_descending;

            entityMap[msg->name()] = e;
        }
    }

    for( const auto* file : files ) {
        for( int m = 0; m < file->message_type_count(); ++m ) {
            const auto* msg = file->message_type(m);
            auto it = entityMap.find(msg->name());
            if( it == entityMap.end() )
                continue;
            auto& e = it->second;

            for( int f = 0; f < msg->field_count(); ++f ) {
                const auto* fd = msg->field(f);
                FieldInfo fi;
                fi.protoName = QString::fromStdString(fd->name());
                fi.getter = toCamelCase(fi.protoName);
                fi.setter = QStringLiteral("set") + toPascalCase(fi.protoName);
                fi.isString = (fd->type() == pb::FieldDescriptor::TYPE_STRING);
                fi.isBytes = (fd->type() == pb::FieldDescriptor::TYPE_BYTES);
                fi.isTimestamp
                    = (fd->type() == pb::FieldDescriptor::TYPE_MESSAGE && fd->message_type()
                       && fd->message_type()->full_name() == "google.protobuf.Timestamp");
                fi.isOptional = fd->has_optional_keyword();
                fi.isRepeated = fd->is_repeated();

                if( fi.protoName == QLatin1String("uid") )
                    e.hasUid = true;
                if( fi.protoName == QLatin1String("key") )
                    e.hasKey = true;
                if( fi.protoName == QLatin1String("created_at") )
                    e.hasCreatedAt = true;
                if( fi.protoName == QLatin1String("updated_at") )
                    e.hasUpdatedAt = true;

                DepsConfigData dc;
                if( getDepsConfig(fd, dc) && !dc.fk.empty() )
                    fi.fkTargets = parseFKAnnotation(dc.fk, entityMap);

                e.allFields.append(fi);

                if( !fi.isOptional && !fi.isRepeated && !fi.isTimestamp && (fi.isString || fi.isBytes) )
                    e.requiredFields.append(fi);

                if( !fi.fkTargets.isEmpty() )
                    e.fkFields.append(fi);
            }

            buildKeyFunctions(e);
        }
    }

    std::vector<EntityInfo> result;
    for( auto& [name, entity] : entityMap )
        result.push_back(std::move(entity));

    std::sort(result.begin(), result.end(), [](const EntityInfo& a, const EntityInfo& b) {
        return a.keyPrefix < b.keyPrefix;
    });

    return result;
}

QVariantMap FKTarget::toVariantMap() const
{
    QVariantMap tm;
    tm[QStringLiteral("entity")] = entity;
    tm[QStringLiteral("field")] = field;
    tm[QStringLiteral("key_prefix")] = keyPrefix;
    return tm;
}

QVariantMap FieldInfo::toVariantMapForFk() const
{
    QVariantMap fm;
    fm[QStringLiteral("proto_name")] = protoName;
    fm[QStringLiteral("getter")] = getter;
    QVariantList targets;
    for( const auto& t : fkTargets )
        targets.append(t.toVariantMap());
    fm[QStringLiteral("fk_targets")] = targets;
    return fm;
}

QVariantMap EntityInfo::toVariantMap() const
{
    QVariantMap m;
    m[QStringLiteral("proto_source_file")] = protoSourceFile;
    m[QStringLiteral("name")] = name;
    m[QStringLiteral("name_snake")] = nameSnake;
    m[QStringLiteral("name_upper")] = nameUpper;
    m[QStringLiteral("name_plural")] = namePlural;
    m[QStringLiteral("cpp_type")] = cppType;
    m[QStringLiteral("key_prefix")] = keyPrefix;
    m[QStringLiteral("key_pattern")] = keyPattern;
    m[QStringLiteral("singleton")] = singleton;
    m[QStringLiteral("persona_scoped")] = personaScoped;
    m[QStringLiteral("sort_field")] = sortField;
    m[QStringLiteral("sort_descending")] = sortDescending;
    m[QStringLiteral("has_uid")] = hasUid;
    m[QStringLiteral("has_key")] = hasKey;
    m[QStringLiteral("has_created_at")] = hasCreatedAt;
    m[QStringLiteral("has_updated_at")] = hasUpdatedAt;
    m[QStringLiteral("const_name")] = constName;
    m[QStringLiteral("fn_name")] = fnName;
    m[QStringLiteral("key_fn_params")] = keyFnParams;
    m[QStringLiteral("key_fn_body")] = keyFnBody;

    QVariantList prefixOverloadList;
    for( const auto& po : prefixOverloads ) {
        QVariantMap pom;
        pom[QStringLiteral("param_count")] = po.paramCount;
        pom[QStringLiteral("params")] = po.params;
        pom[QStringLiteral("body")] = po.body;
        prefixOverloadList.append(pom);
    }
    m[QStringLiteral("prefix_overloads")] = prefixOverloadList;

    QVariantList requiredFieldMaps;
    for( const auto& f : requiredFields ) {
        QVariantMap fm;
        fm[QStringLiteral("proto_name")] = f.protoName;
        fm[QStringLiteral("getter")] = f.getter;
        fm[QStringLiteral("is_string")] = f.isString;
        fm[QStringLiteral("is_bytes")] = f.isBytes;
        requiredFieldMaps.append(fm);
    }
    m[QStringLiteral("required_fields")] = requiredFieldMaps;

    QVariantList fkFieldMaps;
    for( const auto& f : fkFields )
        fkFieldMaps.append(f.toVariantMapForFk());
    m[QStringLiteral("fk_fields")] = fkFieldMaps;

    return m;
}

std::vector<EntityInfo> entitiesInFile(const std::vector<EntityInfo>& all, const std::string& protoPath)
{
    const QString want = QString::fromStdString(protoPath);
    std::vector<EntityInfo> out;
    for( const auto& e : all ) {
        if( e.protoSourceFile == want )
            out.push_back(e);
    }
    return out;
}

} // namespace asocial_protogen
