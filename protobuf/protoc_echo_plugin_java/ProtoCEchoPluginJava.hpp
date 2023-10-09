#ifndef PROTOBUF_PROTO_C_ECHO_PLUGIN_JAVA_HPP
#define PROTOBUF_PROTO_C_ECHO_PLUGIN_JAVA_HPP

#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"
#include <memory>

namespace application
{
    struct UnspecifiedServiceId
    {
        std::string service;
    };

    struct UnspecifiedMethodId
    {
        std::string service;
        std::string method;
    };

    class JavaEchoCodeGenerator
        : public google::protobuf::compiler::CodeGenerator
    {
    public:
        bool Generate(const google::protobuf::FileDescriptor* file, const std::string& parameter,
            google::protobuf::compiler::GeneratorContext* generatorContext, std::string* error) const override;
    };

    class JavaGenerator
    {
    public:
        JavaGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer);

    protected:
        void GenerateFieldConstants();

    protected:
        const google::protobuf::ServiceDescriptor& service;
        google::protobuf::io::Printer& printer;
        uint32_t serviceId;
    };

    class JavaServiceGenerator
        : public JavaGenerator
    {
    public:
        JavaServiceGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer);
        JavaServiceGenerator(const JavaServiceGenerator& other) = delete;
        JavaServiceGenerator& operator=(const JavaServiceGenerator& other) = delete;
        ~JavaServiceGenerator() = default;

    private:
        void GenerateClassHeader();
        void GenerateConstructor();
        void GenerateHandle();
        void GenerateAbstractMethods();
        void GenerateClassFooter();
    };

    class JavaServiceProxyGenerator
        : public JavaGenerator
    {
    public:
        JavaServiceProxyGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer);
        JavaServiceProxyGenerator(const JavaServiceGenerator& other) = delete;
        JavaServiceProxyGenerator& operator=(const JavaServiceProxyGenerator& other) = delete;
        ~JavaServiceProxyGenerator() = default;

    private:
        void GenerateClassHeader();
        void GenerateConstructor();
        void GenerateMethods();
        void GenerateClassFooter();
    };

    class JavaEchoGenerator
    {
    public:
        JavaEchoGenerator(google::protobuf::compiler::GeneratorContext* generatorContext, const std::string& name, const google::protobuf::FileDescriptor* file);
        JavaEchoGenerator(const JavaEchoGenerator& other) = delete;
        JavaEchoGenerator& operator=(const JavaEchoGenerator& other) = delete;
        ~JavaEchoGenerator() = default;

    private:
        std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> stream;
        google::protobuf::io::Printer printer;
    };
}

#endif
