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

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>

class MyGenerator : public google::protobuf::compiler::CodeGenerator
{
public:
    bool Generate(
        const google::protobuf::FileDescriptor* file,
        const std::string& parameter,
        google::protobuf::compiler::GeneratorContext* context,
        std::string* error) const override
    {
        // Example: generate a .gen.h file with extra Qt helpers
        auto* out = context->Open(file->name() + ".gen.h");
        google::protobuf::io::Printer printer(out, '$');
        printer.Print("// Generated aSocial helper for $$   name   $$\n", "name", file->name());
        // ... walk file->message_type(i), fields, etc.
        return true;
    }

    uint64_t GetSupportedFeatures() const override
    {
        // Tell protoc this generator supports proto3 optional fields
        return FEATURE_PROTO3_OPTIONAL;
    }
};

int main(int argc, char** argv)
{
    MyGenerator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
