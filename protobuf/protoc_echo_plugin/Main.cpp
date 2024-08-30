#include "google/protobuf/compiler/plugin.h"
#include "protobuf/protoc_echo_plugin/ProtoCEchoPlugin.hpp"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

int main(int argc, char* argv[])
{
#ifdef _MSC_VER
    //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_WNDW);
    //_CrtDbgReport(_CRT_WARN, nullptr, 0, nullptr, "Attach Visual Studio with Debug -> Attach to Process before pressing Retry");
#endif
    application::CppInfraCodeGenerator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
