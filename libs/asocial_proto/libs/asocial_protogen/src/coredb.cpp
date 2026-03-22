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
#include "asocial_protogen/templates.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include <asocial_tpl/Template.h>

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <iostream>
#include <string>
#include <vector>

namespace pb = google::protobuf;

namespace {

bool parseCoreDbPluginParams(
    const std::string& parameter, std::vector<std::string>* entityFiles, std::string* anchorProto, std::string* err)
{
    if( parameter.empty() ) {
        if( err )
            *err = "protoc-gen-coredb requires options: entity_files=a.proto,b.proto";
        return false;
    }
    anchorProto->clear();
    const QString q = QString::fromStdString(parameter);
    for( const QString& part : q.split(QLatin1Char('|'), Qt::SkipEmptyParts) ) {
        const int eq = part.indexOf(QLatin1Char('='));
        if( eq < 0 )
            continue;
        const QString key = part.left(eq).trimmed();
        const QString val = part.mid(eq + 1).trimmed();
        if( key == QLatin1String("entity_files") ) {
            for( const QString& f : val.split(QLatin1Char(','), Qt::SkipEmptyParts) )
                entityFiles->push_back(f.trimmed().toStdString());
        } else if( key == QLatin1String("anchor") ) {
            *anchorProto = val.trimmed().toStdString();
        }
    }
    if( entityFiles->empty() ) {
        if( err )
            *err = "protoc-gen-coredb: entity_files option is required";
        return false;
    }
    if( anchorProto->empty() )
        *anchorProto = entityFiles->front();
    return true;
}

QString commonIncludeFromProtoPath(const QString& protoPath)
{
    const QString dir = QFileInfo(protoPath).path();
    return dir + QStringLiteral("/common.coredb.gen.h");
}

QString qpbIncludeFromProtoPath(const QString& protoPath)
{
    QString p = protoPath;
    p.replace(QStringLiteral(".proto"), QStringLiteral(".qpb.h"));
    return p;
}

QString protoGenHeaderFromProtoPath(QString protoPath)
{
    return protoPath.replace(QStringLiteral(".proto"), QStringLiteral(".coredb.gen.h"));
}

QString protoGenCppFromProtoPath(QString protoPath)
{
    return protoPath.replace(QStringLiteral(".proto"), QStringLiteral(".coredb.gen.cpp"));
}

void writeStream(pb::compiler::GeneratorContext* context, const std::string& path, const std::string& data)
{
    auto* stream = context->Open(path);
    pb::io::CodedOutputStream coded(stream);
    coded.WriteRaw(data.data(), static_cast<int>(data.size()));
}

} // namespace

class CoreDBGeneratorDynamic : public pb::compiler::CodeGenerator
{
public:
    bool Generate(
        const pb::FileDescriptor* file,
        const std::string& parameter,
        pb::compiler::GeneratorContext* context,
        std::string* error) const override
    {
        (void) error;
        std::vector<std::string> entityPaths;
        std::string anchorPath;
        if( !parseCoreDbPluginParams(parameter, &entityPaths, &anchorPath, nullptr) )
            return false;

        const pb::DescriptorPool* pool = file->pool();
        std::vector<const pb::FileDescriptor*> allFiles;
        allFiles.reserve(entityPaths.size());
        for( const auto& path : entityPaths ) {
            const pb::FileDescriptor* fd = pool->FindFileByName(path);
            if( fd )
                allFiles.push_back(fd);
            else
                std::cerr << "Unable to find entity in the pool:" << path << std::endl;
        }

        const QString currentProto = QString::fromStdString(file->name());
        const bool isAnchorPass = (currentProto == QString::fromStdString(anchorPath));

        const auto entities = asocial_protogen::buildEntities(allFiles);
        const auto fileEntities = asocial_protogen::entitiesInFile(entities, file->name());

        QVariantList entityList;
        entityList.reserve(static_cast<int>(fileEntities.size()));
        for( const auto& e : fileEntities )
            entityList.append(e.toVariantMap());

        const QString commonInc = commonIncludeFromProtoPath(currentProto);
        const QString qpbInc = qpbIncludeFromProtoPath(currentProto);
        const QString selfHeaderInc = protoGenHeaderFromProtoPath(currentProto);

        QVariantMap entityCtx;
        entityCtx[QStringLiteral("entities")] = entityList;
        entityCtx[QStringLiteral("common_include_path")] = commonInc;
        entityCtx[QStringLiteral("qpb_include_path")] = qpbInc;

        QString entityHeaderOut
            = asocial_tpl::Template::renderString(QString::fromUtf8(asocial_protogen::entityHeaderTemplate()), entityCtx);
        writeStream(context, protoGenHeaderFromProtoPath(currentProto).toStdString(), entityHeaderOut.toStdString());

        QVariantMap entitySrcCtx = entityCtx;
        entitySrcCtx[QStringLiteral("header_include_path")] = selfHeaderInc;
        QString entityCppOut = asocial_tpl::Template::renderString(
            QString::fromUtf8(asocial_protogen::entitySourceTemplate()), entitySrcCtx);
        writeStream(context, protoGenCppFromProtoPath(currentProto).toStdString(), entityCppOut.toStdString());

        // Protoc forbids opening the same output twice in one invocation; shared files are
        // emitted only on the anchor pass.
        if( !isAnchorPass )
            return true;

        const QString anchorProto = QString::fromStdString(anchorPath);
        const QDir anchorDir = QFileInfo(anchorProto).dir();
        const std::string coredbHeaderRel = anchorDir.filePath(QStringLiteral("coredb.gen.h")).toStdString();
        const std::string commonHeaderRel = anchorDir.filePath(QStringLiteral("common.coredb.gen.h")).toStdString();
        const std::string commonCppRel = anchorDir.filePath(QStringLiteral("common.coredb.gen.cpp")).toStdString();
        const QString commonIncAnchor = commonIncludeFromProtoPath(anchorProto);

        QVariantMap commonCtx;
        commonCtx[QStringLiteral("common_include_path")] = commonIncAnchor;

        QString commonHeaderOut
            = asocial_tpl::Template::renderString(QString::fromUtf8(asocial_protogen::commonHeaderTemplate()), commonCtx);
        writeStream(context, commonHeaderRel, commonHeaderOut.toStdString());

        QString commonCppOut
            = asocial_tpl::Template::renderString(QString::fromUtf8(asocial_protogen::commonSourceTemplate()), commonCtx);
        writeStream(context, commonCppRel, commonCppOut.toStdString());

        QVariantList typeIncludes;
        for( const auto& p : entityPaths ) {
            const QString inc = protoGenHeaderFromProtoPath(QString::fromStdString(p));
            typeIncludes.append(inc);
        }
        QVariantMap aggCtx;
        aggCtx[QStringLiteral("common_include_path")] = commonIncAnchor;
        aggCtx[QStringLiteral("type_includes")] = typeIncludes;

        QString aggHeaderOut = asocial_tpl::Template::renderString(
            QString::fromUtf8(asocial_protogen::coredbAggregatorHeaderTemplate()), aggCtx);
        writeStream(context, coredbHeaderRel, aggHeaderOut.toStdString());

        QVariantMap aggCppCtx;
        aggCppCtx[QStringLiteral("coredb_header_include_path")] = QString::fromStdString(coredbHeaderRel);
        QString aggCppOut = asocial_tpl::Template::renderString(
            QString::fromUtf8(asocial_protogen::coredbAggregatorSourceTemplate()), aggCppCtx);
        writeStream(context, anchorDir.filePath(QStringLiteral("coredb.gen.cpp")).toStdString(), aggCppOut.toStdString());

        return true;
    }

    uint64_t GetSupportedFeatures() const override { return FEATURE_PROTO3_OPTIONAL; }
};

int main(int argc, char** argv)
{
    CoreDBGeneratorDynamic generator;
    return pb::compiler::PluginMain(argc, argv, &generator);
}
