#include "protobuf/protoc_echo_plugin_csharp/ProtoCEchoPluginCSharp.hpp"
#include "generated/proto_cpp/EchoAttributes.pb.h"
#include "google/protobuf/compiler/cpp/cpp_helpers.h"
#include "google/protobuf/compiler/plugin.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/strutil.h"
#include <sstream>

namespace application
{
    namespace
    {
        std::string UnderscoresToCamelCase(const std::string& input,
            bool cap_next_letter,
            bool preserve_period = true)
        {
            std::string result;
            // Note:  I distrust ctype.h due to locales.
            for (int i = 0; i < input.size(); i++)
            {
                if ('a' <= input[i] && input[i] <= 'z')
                {
                    if (cap_next_letter)
                    {
                        result += input[i] + ('A' - 'a');
                    }
                    else
                    {
                        result += input[i];
                    }
                    cap_next_letter = false;
                }
                else if ('A' <= input[i] && input[i] <= 'Z')
                {
                    if (i == 0 && !cap_next_letter)
                    {
                        // Force first letter to lower-case unless explicitly told to
                        // capitalize it.
                        result += input[i] + ('a' - 'A');
                    }
                    else
                    {
                        // Capital letters after the first are left as-is.
                        result += input[i];
                    }
                    cap_next_letter = false;
                }
                else if ('0' <= input[i] && input[i] <= '9')
                {
                    result += input[i];
                    cap_next_letter = true;
                }
                else
                {
                    cap_next_letter = true;
                    if (input[i] == '.' && preserve_period)
                    {
                        result += '.';
                    }
                }
            }
            // Add a trailing "_" if the name should be altered.
            if (input[input.size() - 1] == '#')
            {
                result += '_';
            }
            return result;
        }

        std::string QualifiedName(const google::protobuf::Descriptor& descriptor)
        {
            std::string namespaceString;

            if (!descriptor.file()->package().empty())
                namespaceString = UnderscoresToCamelCase(descriptor.file()->package(), true) + ".";
            for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
                namespaceString += UnderscoresToCamelCase(containingType->name(), true) + ".";

            return namespaceString + descriptor.name();
        }
    }

    bool CSharpEchoCodeGenerator::Generate(const google::protobuf::FileDescriptor* file, const std::string& parameter,
        google::protobuf::compiler::GeneratorContext* generatorContext, std::string* error) const
    {
        try
        {
            std::string basename = google::protobuf::compiler::cpp::StripProto(file->name()) + ".pb.cs";

            CSharpEchoGenerator generator(generatorContext, basename, file);

            return true;
        }
        catch (UnspecifiedServiceId& exception)
        {
            *error = "Field " + exception.service + " needs a service_id specifying its id";
            return false;
        }
        catch (UnspecifiedMethodId& exception)
        {
            *error = "Field " + exception.service + "." + exception.method + " needs a method_id specifying its id";
            return false;
        }
    }

    CSharpGenerator::CSharpGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer)
        : service(service)
        , printer(printer)
        , serviceId(service.options().GetExtension(service_id))
    {}

    void CSharpGenerator::GenerateFieldConstants()
    {
        printer.Print(R"(        const int serviceId = $id$;

)",
            "id", google::protobuf::SimpleItoa(serviceId));

        for (int i = 0; i != service.method_count(); ++i)
        {
            uint32_t methodId = service.method(i)->options().GetExtension(method_id);
            if (methodId == 0)
                throw UnspecifiedMethodId{ service.name(), service.method(i)->name() };

            printer.Print("        const int id$method$ = $id$;\n", "method", service.method(i)->name(), "id", google::protobuf::SimpleItoa(methodId));
        }

        printer.Print("\n");
    }

    CSharpServiceGenerator::CSharpServiceGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer)
        : CSharpGenerator(service, printer)
    {
        if (serviceId == 0)
            throw UnspecifiedServiceId{ service.name() };

        GenerateClassHeader();
        GenerateFieldConstants();
        GenerateDelegates();
        GenerateConstructor();
        GenerateHandle();
        GenerateClassFooter();
    }

    void CSharpServiceGenerator::GenerateClassHeader()
    {
        printer.Print(R"(    public class $name$
        : Service
    {
)",
            "name", service.name());
    }

    void CSharpServiceGenerator::GenerateDelegates()
    {
        for (int i = 0; i != service.method_count(); ++i)
        {
            if (service.method(i)->input_type()->full_name() != Nothing::descriptor()->full_name())
                printer.Print("        public delegate void $method$Delegate(global::$parameter$ parameter);\n", "method", service.method(i)->name(), "parameter", QualifiedName(*service.method(i)->input_type()));
            else
                printer.Print("        public delegate void $method$Delegate();\n", "method", service.method(i)->name());
            printer.Print("        public $method$Delegate $method_var$;\n", "method", service.method(i)->name(), "method_var", service.method(i)->name());
        }

        printer.Print("\n");
    }

    void CSharpServiceGenerator::GenerateConstructor()
    {
        printer.Print(R"(        public $name$(Echo echo)
            : base(echo, serviceId)
        {}

)",
            "name", service.name());
    }

    void CSharpServiceGenerator::GenerateHandle()
    {
        printer.Print(R"(        public override void Handle(int methodId, pb.CodedInputStream stream)
        {
            switch (methodId)
            {
)");

        for (int i = 0; i != service.method_count(); ++i)
        {
            if (service.method(i)->input_type()->full_name() != Nothing::descriptor()->full_name())
                printer.Print(R"(                case id$method_type$:
                {
                    var parameter = new global::$parameter_type$();
                    parameter.MergeFrom(stream);
                    $method$?.Invoke(parameter);
                    break;
                }
)",
                    "method_type", service.method(i)->name(), "method", service.method(i)->name(), "parameter_type", QualifiedName(*service.method(i)->input_type()));
            else
                printer.Print(R"(                case id$method_type$:
                {
                    $method$?.Invoke();
                    break;
                }
)",
                    "method_type", service.method(i)->name(), "method", service.method(i)->name());
        }

        printer.Print(R"(            }
        }
)");
    }

    void CSharpServiceGenerator::GenerateClassFooter()
    {
        printer.Print(R"(    }

)");
    }

    CSharpServiceProxyGenerator::CSharpServiceProxyGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer)
        : CSharpGenerator(service, printer)
    {
        if (serviceId == 0)
            throw UnspecifiedServiceId{ service.name() };

        GenerateClassHeader();
        GenerateFieldConstants();
        GenerateConstructor();
        GenerateMethods();
        GenerateClassFooter();
    }

    void CSharpServiceProxyGenerator::GenerateClassHeader()
    {
        printer.Print(R"(    public class $name$Proxy
        : ServiceProxy
    {
)",
            "name", service.name());
    }

    void CSharpServiceProxyGenerator::GenerateConstructor()
    {
        printer.Print(R"(        public $name$Proxy(Echo echo)
            : base(echo)
        {}

)",
            "name", service.name());
    }

    void CSharpServiceProxyGenerator::GenerateMethods()
    {
        for (int i = 0; i != service.method_count(); ++i)
        {
            if (service.method(i)->input_type()->full_name() != Nothing::descriptor()->full_name())
                printer.Print(R"(        public void $method$(global::$parameter_type$ parameter)
)",
                    "method", service.method(i)->name(), "parameter_type", QualifiedName(*service.method(i)->input_type()));
            else
                printer.Print(R"(        public void $method$()
)",
                    "method", service.method(i)->name());
            printer.Print(R"(        {
            var stream = echo.GetOutputStream();

            stream.WriteInt32(serviceId);
            stream.WriteUInt32(pb.WireFormat.MakeTag(id$method$, pb.WireFormat.WireType.LengthDelimited));
)",
                "method", service.method(i)->name());
            if (service.method(i)->input_type()->full_name() != Nothing::descriptor()->full_name())
                printer.Print(R"(            stream.WriteMessage(parameter);
)");
            else
                printer.Print(R"(            stream.WriteUInt32(0);
)");
            printer.Print(R"(
            echo.SendOutputStream(stream);
        }

)");
        }
    }

    void CSharpServiceProxyGenerator::GenerateClassFooter()
    {
        printer.Print(R"(    }

)");
    }

    CSharpEchoGenerator::CSharpEchoGenerator(google::protobuf::compiler::GeneratorContext* generatorContext, const std::string& name, const google::protobuf::FileDescriptor* file)
        : stream(generatorContext->Open(name))
        , printer(stream.get(), '$', nullptr)
    {
        printer.Print(R"(// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: $filename$

using ProtobufEcho;
using pb = Google.Protobuf;

namespace IntegrationTestFramework.ProtobufEcho
{
)",
            "filename", file->name());

        for (int i = 0; i != file->service_count(); ++i)
        {
            CSharpServiceGenerator(*file->service(i), printer);
            CSharpServiceProxyGenerator(*file->service(i), printer);
        }

        printer.Print("}\n");
    }
}
