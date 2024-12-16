#ifndef PROTOBUF_PROTO_C_ECHO_PLUGIN_C_SHARP_HPP
#define PROTOBUF_PROTO_C_ECHO_PLUGIN_C_SHARP_HPP

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

    class CSharpEchoCodeGenerator
        : public google::protobuf::compiler::CodeGenerator
    {
    public:
        bool Generate(const google::protobuf::FileDescriptor* file, const std::string& parameter,
            google::protobuf::compiler::GeneratorContext* generatorContext, std::string* error) const override;

        uint64_t GetSupportedFeatures() const override;
    };

    class CSharpGenerator
    {
    public:
        CSharpGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer);

    protected:
        void GenerateFieldConstants();

    protected:
        const google::protobuf::ServiceDescriptor& service;
        google::protobuf::io::Printer& printer;
        uint32_t serviceId;
    };

    class CSharpServiceGenerator
        : public CSharpGenerator
    {
    public:
        CSharpServiceGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer);
        CSharpServiceGenerator(const CSharpServiceGenerator& other) = delete;
        CSharpServiceGenerator& operator=(const CSharpServiceGenerator& other) = delete;
        ~CSharpServiceGenerator() = default;

    private:
        void GenerateClassHeader();
        void GenerateDelegates();
        void GenerateConstructor();
        void GenerateHandle();
        void GenerateClassFooter();
    };

    class CSharpServiceProxyGenerator
        : public CSharpGenerator
    {
    public:
        CSharpServiceProxyGenerator(const google::protobuf::ServiceDescriptor& service, google::protobuf::io::Printer& printer);
        CSharpServiceProxyGenerator(const CSharpServiceGenerator& other) = delete;
        CSharpServiceProxyGenerator& operator=(const CSharpServiceProxyGenerator& other) = delete;
        ~CSharpServiceProxyGenerator() = default;

    private:
        void GenerateClassHeader();
        void GenerateConstructor();
        void GenerateMethods();
        void GenerateClassFooter();
    };

    class CSharpEchoGenerator
    {
    public:
        CSharpEchoGenerator(google::protobuf::compiler::GeneratorContext* generatorContext, const std::string& name, const google::protobuf::FileDescriptor* file);
        CSharpEchoGenerator(const CSharpEchoGenerator& other) = delete;
        CSharpEchoGenerator& operator=(const CSharpEchoGenerator& other) = delete;
        ~CSharpEchoGenerator() = default;

    private:
        std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> stream;
        google::protobuf::io::Printer printer;
    };
}

#endif
