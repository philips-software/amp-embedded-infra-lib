#ifndef PROTOBUF_PROTO_C_ECHO_PLUGIN_HPP
#define PROTOBUF_PROTO_C_ECHO_PLUGIN_HPP

#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/io/printer.h"
#include "protobuf/protoc_echo_plugin/CppFormatter.hpp"
#include "protobuf/protoc_echo_plugin/EchoObjects.hpp"

namespace application
{
    class CppInfraCodeGenerator
        : public google::protobuf::compiler::CodeGenerator
    {
    public:
        virtual bool Generate(const google::protobuf::FileDescriptor* file, const std::string& parameter,
            google::protobuf::compiler::GeneratorContext* generatorContext, std::string* error) const override;
    };

    class MessageGenerator
    {
    public:
        MessageGenerator(const std::shared_ptr<const EchoMessage>& message, Entities& formatter);
        MessageGenerator(const MessageGenerator& other) = delete;
        MessageGenerator& operator=(const MessageGenerator& other) = delete;
        ~MessageGenerator() = default;

    private:
        void GenerateConstructors();
        void GenerateFunctions();
        void GenerateNestedMessageForwardDeclarations();
        void GenerateNestedMessages();
        void GenerateFieldDeclarations();
        void GenerateFieldConstants();
        void GenerateMaxMessageSize();
        std::string SerializerBody();
        std::string DeserializerBody();
        std::string CompareEqualBody() const;
        std::string CompareUnEqualBody() const;

    private:
        std::shared_ptr<const EchoMessage> message;
        Class* classFormatter;
        std::vector<std::shared_ptr<MessageGenerator>> messageGenerators;
    };

    class ServiceGenerator
    {
    public:
        ServiceGenerator(const std::shared_ptr<const EchoService>& service, Entities& formatter);
        ServiceGenerator(const ServiceGenerator& other) = delete;
        ServiceGenerator& operator=(const ServiceGenerator& other) = delete;
        ~ServiceGenerator() = default;

    private:
        void GenerateServiceConstructors();
        void GenerateServiceProxyConstructors();
        void GenerateServiceFunctions();
        void GenerateServiceProxyFunctions();
        void GenerateFieldConstants();

        std::string MaxMessageSize() const;
        std::string HandleBody() const;
        std::string ProxyMethodBody(const EchoMethod& method) const;

    private:
        std::shared_ptr<const EchoService> service;
        Class* serviceFormatter;
        Class* serviceProxyFormatter;
    };

    class EchoGenerator
    {
    public:
        EchoGenerator(google::protobuf::compiler::GeneratorContext* generatorContext, const std::string& name, const google::protobuf::FileDescriptor* file);
        EchoGenerator(const EchoGenerator& other) = delete;
        EchoGenerator& operator=(const EchoGenerator& other) = delete;
        ~EchoGenerator() = default;

    public:
        void GenerateHeader();
        void GenerateSource();

    private:
        void GenerateTopHeaderGuard();
        void GenerateBottomHeaderGuard();

    private:
        google::protobuf::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream> stream;
        google::protobuf::io::Printer printer;
        Entities formatter;
        const google::protobuf::FileDescriptor* file;

        std::vector<std::shared_ptr<MessageGenerator>> messageGenerators;
        std::vector<std::shared_ptr<ServiceGenerator>> serviceGenerators;
    };
}

#endif
