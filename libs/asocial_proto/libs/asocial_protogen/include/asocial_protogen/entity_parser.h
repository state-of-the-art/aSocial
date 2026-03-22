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

#ifndef ASOCIAL_PROTOGEN_ENTITY_PARSER_H
#define ASOCIAL_PROTOGEN_ENTITY_PARSER_H

#include <map>
#include <string>
#include <vector>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace google::protobuf {
class Descriptor;
class FieldDescriptor;
class FileDescriptor;
} // namespace google::protobuf

namespace asocial_protogen {

class FKTarget
{
public:
    QString entity;
    QString field;
    QString keyPrefix;

    QVariantMap toVariantMap() const;
};

class FieldInfo
{
public:
    QString protoName;
    QString getter;
    QString setter;
    bool isString = false;
    bool isBytes = false;
    bool isTimestamp = false;
    bool isOptional = false;
    bool isRepeated = false;
    QList<FKTarget> fkTargets;

    /// For `fk_fields` template (proto, getter, fk_targets).
    QVariantMap toVariantMapForFk() const;
};

class KeySegment
{
public:
    QString name;
    QString paramName;
    bool isTimestamp = false;
};

class PrefixOverload
{
public:
    int paramCount = 0;
    QString params;
    QString body;
};

class EntityInfo
{
public:
    /// Descriptor pool path, e.g. asocial/v1/contact.proto
    QString protoSourceFile;
    QString name;
    QString nameSnake;
    QString nameUpper;
    QString namePlural;
    QString cppType;
    QString keyPrefix;
    QString keyPattern;
    bool singleton = false;
    bool personaScoped = false;
    QString sortField;
    bool sortDescending = false;

    bool hasUid = false;
    bool hasCreatedAt = false;
    bool hasUpdatedAt = false;
    bool hasKey = false;

    QList<FieldInfo> allFields;
    QList<FieldInfo> requiredFields;
    QList<FieldInfo> fkFields;

    QString keyFnParams;
    QString keyFnBody;

    QList<PrefixOverload> prefixOverloads;
    QString constName;
    QString fnName;

    QVariantMap toVariantMap() const;
};

std::vector<EntityInfo> buildEntities(const std::vector<const google::protobuf::FileDescriptor*>& files);

std::vector<EntityInfo> entitiesInFile(const std::vector<EntityInfo>& all, const std::string& protoPath);

} // namespace asocial_protogen

#endif // ASOCIAL_PROTOGEN_ENTITY_PARSER_H
