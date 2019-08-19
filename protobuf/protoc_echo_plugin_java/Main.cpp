#include "google/protobuf/compiler/plugin.h"
#include "protobuf/protoc_echo_plugin_java/ProtoCEchoPluginJava.hpp"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

int main(int argc, char* argv[])
{
#ifdef _MSC_VER
    //_CrtDbgBreak();
#endif
    application::JavaEchoCodeGenerator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
