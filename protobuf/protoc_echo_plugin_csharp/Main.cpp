#include "google/protobuf/compiler/plugin.h"
#include "protobuf/protoc_echo_plugin_csharp/ProtoCEchoPluginCSharp.hpp"
#include <crtdbg.h>

int main(int argc, char* argv[])
{
    //_CrtDbgBreak();
    application::CSharpEchoCodeGenerator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
