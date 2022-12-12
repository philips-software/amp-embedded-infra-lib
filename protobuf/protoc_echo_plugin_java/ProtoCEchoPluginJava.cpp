#include "protobuf/protoc_echo_plugin_java/ProtoCEchoPluginJava.hpp"
#include "generated/EchoAttributes.pb.h"
#include "google/protobuf/compiler/cpp/helpers.h"
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

            for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
                namespaceString += UnderscoresToCamelCase(containingType->name(), false) + ".";

            return namespaceString + descriptor.name();
        }
    }

    bool JavaEchoCodeGenerator::Generate(const google::protobuf::FileDescriptor* file, const std::string& parameter,
        google::protobuf::compiler::GeneratorContext* generatorContext, std::string* error) const
    {
        try
        {
            std::string basename = google::protobuf::compiler::cpp::StripProto(file->name()) + "Services.java";

            JavaEchoGenerator generator(generatorContext, basename, file);

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

    JavaGenerator::JavaGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer)
        : service(service)
        , printer(printer)
        , serviceId(service.options().GetExtension(service_id))
    {}

    void JavaGenerator::GenerateFieldConstants()
    {
        printer.Print(R"(        final static int serviceId = $id$;

)",
            "id", google::protobuf::SimpleItoa(serviceId));

        for (int i = 0; i != service.method_count(); ++i)
        {
            uint32_t methodId = service.method(i)->options().GetExtension(method_id);
            if (methodId == 0)
                throw UnspecifiedMethodId{ service.name(), service.method(i)->name() };

            printer.Print("        final static int id$method$ = $id$;\n", "method", service.method(i)->name(), "id", google::protobuf::SimpleItoa(methodId));
        }

        printer.Print("\n");
    }

    JavaServiceGenerator::JavaServiceGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer)
        : JavaGenerator(service, printer)
    {
        if (serviceId == 0)
            throw UnspecifiedServiceId{ service.name() };

        GenerateClassHeader();
        GenerateFieldConstants();
        GenerateConstructor();
        GenerateHandle();
        GenerateAbstractMethods();
        GenerateClassFooter();
    }

    void JavaServiceGenerator::GenerateClassHeader()
    {
        printer.Print(R"(    public static abstract class $name$ extends Service {
)",
            "name", service.name());
    }

    void JavaServiceGenerator::GenerateConstructor()
    {
        printer.Print(R"(        public $name$(Echo echo) {
            super(echo, serviceId);
        }

)",
            "name", service.name());
    }

    void JavaServiceGenerator::GenerateHandle()
    {
        printer.Print(R"(        @Override
        public void handle(int methodId, CodedInputStream stream) throws IOException {
            switch (methodId) {
)");

        for (int i = 0; i != service.method_count(); ++i)
        {
            if (service.method(i)->input_type()->full_name() != Nothing::descriptor()->full_name())
                printer.Print(R"(                case id$method_type$:
                    $parameter_type$ param$parameter_nr$ = $parameter_type$.newBuilder().mergeFrom(stream).build();
                    $method$(param$parameter_nr$);
                    break;
)",
                    "method_type", service.method(i)->name(), "method", UnderscoresToCamelCase(service.method(i)->name(), false), "parameter_type", QualifiedName(*service.method(i)->input_type()), "parameter_nr", google::protobuf::SimpleItoa(i));
            else
                printer.Print(R"(                case id$method_type$:
                    $method$();
                    break;
)",
                    "method_type", service.method(i)->name(), "method", UnderscoresToCamelCase(service.method(i)->name(), false));
        }

        printer.Print(R"(            }
        }
)");
    }

    void JavaServiceGenerator::GenerateAbstractMethods()
    {
        for (int i = 0; i != service.method_count(); ++i)
        {
            if (service.method(i)->input_type()->full_name() != Nothing::descriptor()->full_name())
                printer.Print(R"(
        public abstract void $method$($parameter$ parameter);
)",
                    "method", UnderscoresToCamelCase(service.method(i)->name(), false), "parameter", QualifiedName(*service.method(i)->input_type()));
            else
                printer.Print(R"(
        public abstract void $method$();
)",
                    "method", UnderscoresToCamelCase(service.method(i)->name(), false));
        }
    }

    void JavaServiceGenerator::GenerateClassFooter()
    {
        printer.Print(R"(    }

)");
    }

    JavaServiceProxyGenerator::JavaServiceProxyGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer)
        : JavaGenerator(service, printer)
    {
        if (serviceId == 0)
            throw UnspecifiedServiceId{ service.name() };

        GenerateClassHeader();
        GenerateFieldConstants();
        GenerateConstructor();
        GenerateMethods();
        GenerateClassFooter();
    }

    void JavaServiceProxyGenerator::GenerateClassHeader()
    {
        printer.Print(R"(    public static class $name$Proxy extends ServiceProxy {
)",
            "name", service.name());
    }

    void JavaServiceProxyGenerator::GenerateConstructor()
    {
        printer.Print(R"(        public $name$Proxy(Echo echo) {
            super(echo);
        }

)",
            "name", service.name());
    }

    void JavaServiceProxyGenerator::GenerateMethods()
    {
        for (int i = 0; i != service.method_count(); ++i)
        {
            if (service.method(i)->input_type()->full_name() != Nothing::descriptor()->full_name())
                printer.Print(R"(        public void $method$($parameter_type$ parameter) throws IOException
)",
                    "method", UnderscoresToCamelCase(service.method(i)->name(), false), "parameter_type", QualifiedName(*service.method(i)->input_type()));
            else
                printer.Print(R"(        public void $method$() throws IOException
)",
                    "method", UnderscoresToCamelCase(service.method(i)->name(), false));
            printer.Print(R"(        {
            CodedOutputStream stream = echo.getOutputStream();

            stream.writeInt32NoTag(serviceId);
            stream.writeTag(id$method$, WireFormat.WIRETYPE_LENGTH_DELIMITED);
)",
                "method", service.method(i)->name());
            if (service.method(i)->input_type()->full_name() != Nothing::descriptor()->full_name())
                printer.Print(R"(            stream.writeMessageNoTag(parameter);
)");
            else
                printer.Print(R"(            stream.writeUInt32NoTag(0);
)");
            printer.Print(R"(
            echo.sendOutputStream(stream);
        }

)");
        }
    }

    void JavaServiceProxyGenerator::GenerateClassFooter()
    {
        printer.Print(R"(    }

)");
    }

    JavaEchoGenerator::JavaEchoGenerator(google::protobuf::compiler::GeneratorContext* generatorContext, const std::string& name, const google::protobuf::FileDescriptor* file)
        : stream(generatorContext->Open(name))
        , printer(stream.get(), '$', nullptr)
    {
        printer.Print(R"(// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: $filename$
package com.philips.emil.protobufEcho;

import com.google.protobuf.*;

import java.io.IOException;

public class $basename$Services {

    private $basename$Services() {}

)",
            "filename", file->name(), "basename", google::protobuf::compiler::cpp::StripProto(file->name()));

        for (int i = 0; i != file->service_count(); ++i)
        {
            JavaServiceGenerator(*file->service(i), printer);
            JavaServiceProxyGenerator(*file->service(i), printer);
        }

        printer.Print("\n}\n");
    }
}
