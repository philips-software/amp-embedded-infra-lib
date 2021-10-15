#include "generated/proto_cpp/EchoAttributes.pb.h"
#include "google/protobuf/compiler/cpp/cpp_helpers.h"
#include "google/protobuf/compiler/plugin.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/strutil.h"
#include "protobuf/protoc_echo_plugin/ProtoCEchoPlugin.hpp"
#include <sstream>

namespace application
{
    namespace
    {
        class StorageTypeVisitor
            : public EchoFieldVisitor
        {
        public:
            StorageTypeVisitor(std::string& result)
                : result(result)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                result = "int64_t";
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                result = "uint64_t";
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                result = "int32_t";
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                result = "uint64_t";
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                result = "uint32_t";
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                result = "bool";
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                result = "infra::BoundedString::WithStorage<" + google::protobuf::SimpleItoa(field.maxStringSize) + ">";
            }

            virtual void VisitUnboundedString(const EchoFieldUnboundedString& field) override
            {
                result = "std::string";
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                result = field.type->qualifiedDetailName;
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                result = "int64_t";
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                result = "int32_t";
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                result = field.message->qualifiedDetailName;
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                result = "infra::BoundedVector<uint8_t>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxBytesSize) + ">";
            }

            virtual void VisitUnboundedBytes(const EchoFieldUnboundedBytes& field) override
            {
                result = "std::vector<uint8_t>";
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                result = "uint32_t";
            }

            virtual void VisitRepeated(const EchoFieldRepeated& field) override
            {
                std::string r;
                StorageTypeVisitor visitor(r);
                field.type->Accept(visitor);
                result = "infra::BoundedVector<" + r + ">::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">";
            }

            virtual void VisitUnboundedRepeated(const EchoFieldUnboundedRepeated& field) override
            {
                std::string r;
                StorageTypeVisitor visitor(r);
                field.type->Accept(visitor);
                result = "std::vector<" + r + ">";
            }

        protected:
            std::string& result;
        };

        class ReferenceStorageTypeVisitor
            : public StorageTypeVisitor
        {
        public:
            using StorageTypeVisitor::StorageTypeVisitor;

            virtual void VisitString(const EchoFieldString& field) override
            {
                result = "infra::BoundedConstString";
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                result = field.message->qualifiedDetailReferenceName;
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                result = "infra::ConstByteRange";
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                result = field.type->name;
            }

            virtual void VisitRepeated(const EchoFieldRepeated& field) override
            {
                std::string r;
                ReferenceStorageTypeVisitor visitor(r);
                field.type->Accept(visitor);
                result = "infra::BoundedVector<" + r + ">::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">";
            }
        };

        class ParameterTypeVisitor
            : public StorageTypeVisitor
        {
        public:
            using StorageTypeVisitor::StorageTypeVisitor;

            virtual void VisitString(const EchoFieldString& field) override
            {
                result = "infra::BoundedConstString";
            }

            virtual void VisitUnboundedString(const EchoFieldUnboundedString& field) override
            {
                result = "const std::string&";
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                result = "const " + field.message->qualifiedName + "&";
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                result = "const infra::BoundedVector<uint8_t>&";
            }

            virtual void VisitUnboundedBytes(const EchoFieldUnboundedBytes& field) override
            {
                result = "const std::vector<uint8_t>&";
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                result = field.type->qualifiedDetailName;
            }

            virtual void VisitRepeated(const EchoFieldRepeated& field) override
            {
                std::string r;
                StorageTypeVisitor visitor(r);
                field.type->Accept(visitor);
                result = "const infra::BoundedVector<" + r + ">&";
            }

            virtual void VisitUnboundedRepeated(const EchoFieldUnboundedRepeated& field) override
            {
                std::string r;
                StorageTypeVisitor visitor(r);
                field.type->Accept(visitor);
                result = "const std::vector<" + r + ">&";
            }
        };

        class ParameterReferenceTypeVisitor
            : public ParameterTypeVisitor
        {
        public:
            using ParameterTypeVisitor::ParameterTypeVisitor;

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                result = "const " + field.message->qualifiedDetailReferenceName + "&";
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                result = "infra::ConstByteRange";
            }

            virtual void VisitRepeated(const EchoFieldRepeated& field) override
            {
                std::string r;
                ReferenceStorageTypeVisitor visitor(r);
                field.type->Accept(visitor);
                result = "const infra::BoundedVector<" + r + ">&";
            }
        };
    }

    bool CppInfraCodeGenerator::Generate(const google::protobuf::FileDescriptor* file, const std::string& parameter,
        google::protobuf::compiler::GeneratorContext* generatorContext, std::string* error) const
    {
        try
        {
            std::string basename = google::protobuf::compiler::cpp::StripProto(file->name()) + ".pb";

            EchoGenerator headerGenerator(generatorContext, basename + ".hpp", file);
            headerGenerator.GenerateHeader();
            EchoGenerator sourceGenerator(generatorContext, basename + ".cpp", file);
            sourceGenerator.GenerateSource();

            return true;
        }
        catch (UnsupportedFieldType& exception)
        {
            *error = "Unsupported field type " + google::protobuf::SimpleItoa(exception.type) + " of field " + exception.fieldName;
            return false;
        }
        catch (UnspecifiedStringSize& exception)
        {
            *error = "Field " + exception.fieldName + " needs a string_size specifying its maximum number of characters";
            return false;
        }
        catch (UnspecifiedBytesSize& exception)
        {
            *error = "Field " + exception.fieldName + " needs a bytes_size specifying its maximum number of bytes";
            return false;
        }
        catch (UnspecifiedArraySize& exception)
        {
            *error = "Field " + exception.fieldName + " needs an array_size specifying its maximum number of elements";
            return false;
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
        catch (MessageNotFound& exception)
        {
            *error = "Message " + exception.name + " was used before having been fully defined";
            return false;
        }
        catch (EnumNotFound& exception)
        {
            *error = "Enum " + exception.name + " was used before having been fully defined";
            return false;
        }
    }

    EnumGenerator::EnumGenerator(const std::shared_ptr<const EchoEnum>& enum_)
        : enum_(enum_)
    {}

    void EnumGenerator::Run(Entities& formatter)
    {
        formatter.Add(std::make_shared<EnumDeclaration>(enum_->name, enum_->members));
    }

    MessageEnumGenerator::MessageEnumGenerator(const std::shared_ptr<const EchoMessage>& message)
        : message(message)
    {}

    void MessageEnumGenerator::Run(Entities& formatter)
    {
        if (!message->nestedEnums.empty())
        {
            auto enumNamespace = std::make_shared<Namespace>("detail");

            for (auto& nestedEnum : message->nestedEnums)
                enumNamespace->Add(std::make_shared<EnumDeclaration>(message->name + nestedEnum->name, nestedEnum->members));

            formatter.Add(enumNamespace);
        }
    }

    MessageTypeMapGenerator::MessageTypeMapGenerator(const std::shared_ptr<const EchoMessage>& message, const std::string& prefix)
        : message(message)
        , prefix(prefix)
    {}

    void MessageTypeMapGenerator::Run(Entities& formatter)
    {
        auto typeMapNamespace = std::make_shared<Namespace>("detail");

        auto typeMapDeclaration = std::make_shared<StructTemplateForwardDeclaration>(MessageName() + "TypeMap");
        typeMapDeclaration->TemplateParameter("std::size_t fieldIndex");
        typeMapNamespace->Add(typeMapDeclaration);

        for (auto& field : message->fields)
        {
            auto typeMapSpecialization = std::make_shared<StructTemplateSpecialization>(MessageName() + "TypeMap");
            typeMapSpecialization->TemplateSpecialization(google::protobuf::SimpleItoa(std::distance(message->fields.data(), &field)));
            typeMapSpecialization->Add(std::make_shared<Using>("ProtoType", field->protoType));
            AddTypeMapType(*field, *typeMapSpecialization);
            typeMapNamespace->Add(typeMapSpecialization);
        }

        formatter.Add(typeMapNamespace);
    }

    void MessageTypeMapGenerator::AddTypeMapType(EchoField& field, Entities& entities)
    {
        std::string result;
        StorageTypeVisitor visitor(result);
        field.Accept(visitor);
        entities.Add(std::make_shared<Using>("Type", result));
    }

    std::string MessageTypeMapGenerator::MessageName() const
    {
        return prefix + message->name + MessageSuffix();
    }

    std::string MessageTypeMapGenerator::MessageSuffix() const
    {
        return "";
    }

    void MessageReferenceTypeMapGenerator::Run(Entities& formatter)
    {
        auto typeMapNamespace = std::make_shared<Namespace>("detail");

        auto typeMapDeclaration = std::make_shared<StructTemplateForwardDeclaration>(MessageName() + "TypeMap");
        typeMapDeclaration->TemplateParameter("std::size_t fieldIndex");
        typeMapNamespace->Add(typeMapDeclaration);

        for (auto& field : message->fields)
        {
            auto typeMapSpecialization = std::make_shared<StructTemplateSpecialization>(MessageName() + "TypeMap");
            typeMapSpecialization->TemplateSpecialization(google::protobuf::SimpleItoa(std::distance(message->fields.data(), &field)));
            typeMapSpecialization->Add(std::make_shared<Using>("ProtoType", field->protoReferenceType));
            AddTypeMapType(*field, *typeMapSpecialization);
            typeMapNamespace->Add(typeMapSpecialization);
        }

        formatter.Add(typeMapNamespace);
    }

    void MessageReferenceTypeMapGenerator::AddTypeMapType(EchoField& field, Entities& entities)
    {
        std::string result;
        ParameterReferenceTypeVisitor visitor(result);
        field.Accept(visitor);
        entities.Add(std::make_shared<Using>("Type", result));
    }

    std::string MessageReferenceTypeMapGenerator::MessageSuffix() const
    {
        return "Reference";
    }

    MessageGenerator::MessageGenerator(const std::shared_ptr<const EchoMessage>& message, const std::string& prefix)
        : message(message)
        , prefix(prefix)
    {}

    void MessageGenerator::Run(Entities& formatter)
    {
        GenerateNestedMessages(formatter);
        GenerateTypeMap(formatter);
        GenerateClass(formatter);
        GenerateNestedMessageAliases();
        GenerateEnums();
        GenerateConstructors();
        GenerateFunctions();
        GenerateTypeMap();
        GenerateGetters();
        GenerateFieldDeclarations();
        GenerateFieldConstants();
        GenerateMaxMessageSize();
    }

    void MessageGenerator::GenerateTypeMap(Entities& formatter)
    {
        MessageTypeMapGenerator typeMapGenerator(message, prefix);
        typeMapGenerator.Run(formatter);
    }

    void MessageGenerator::GenerateClass(Entities& formatter)
    {
        auto class_ = std::make_shared<Class>(ClassName());
        classFormatter = class_.get();
        formatter.Add(class_);
    }

    void MessageGenerator::GenerateConstructors()
    {
        auto constructors = std::make_shared<Access>("public");
        constructors->Add(std::make_shared<Constructor>(ClassName(), "", Constructor::cDefault));

        if (!message->fields.empty())
        {
            auto constructByMembers = std::make_shared<Constructor>(ClassName(), "", 0);
            std::string typeNameResult;
            ParameterTypeVisitor visitor(typeNameResult);
            for (auto& field : message->fields)
            {
                field->Accept(visitor);
                constructByMembers->Parameter(typeNameResult + " " + field->name);
                constructByMembers->Initializer(field->name + "(" + field->name + ")");
            }
            constructors->Add(constructByMembers);
        }

        auto constructByProtoParser = std::make_shared<Constructor>(ClassName(), "Deserialize(parser);\n", 0);
        constructByProtoParser->Parameter("infra::ProtoParser& parser");
        constructors->Add(constructByProtoParser);
        classFormatter->Add(constructors);
    }

    void MessageGenerator::GenerateFunctions()
    {
        auto functions = std::make_shared<Access>("public");

        auto serialize = std::make_shared<Function>("Serialize", SerializerBody(), "void", Function::fConst);
        serialize->Parameter("infra::ProtoFormatter& formatter");
        functions->Add(serialize);

        auto deserialize = std::make_shared<Function>("Deserialize", DeserializerBody(), "void", 0);
        deserialize->Parameter("infra::ProtoParser& parser");
        functions->Add(deserialize);

        auto compareEqual = std::make_shared<Function>("operator==", CompareEqualBody(), "bool", Function::fConst);
        compareEqual->Parameter("const " + ClassName() + "& other");
        functions->Add(compareEqual);

        auto compareUnEqual = std::make_shared<Function>("operator!=", CompareUnEqualBody(), "bool", Function::fConst);
        compareUnEqual->Parameter("const " + ClassName() + "& other");
        functions->Add(compareUnEqual);

        classFormatter->Add(functions);
    }

    void MessageGenerator::GenerateTypeMap()
    {
        auto typeMap = std::make_shared<Access>("public");

        auto protoTypeUsing = std::make_shared<UsingTemplate>("ProtoType", "typename " + TypeMapName() + "<fieldIndex>::ProtoType");
        protoTypeUsing->TemplateParameter("std::size_t fieldIndex");
        typeMap->Add(protoTypeUsing);
        auto typeUsing = std::make_shared<UsingTemplate>("Type", "typename " + TypeMapName() + "<fieldIndex>::Type");
        typeUsing->TemplateParameter("std::size_t fieldIndex");
        typeMap->Add(typeUsing);

        classFormatter->Add(typeMap);
    }

    void MessageGenerator::GenerateGetters()
    {
        auto getters = std::make_shared<Access>("public");

        for (auto& field : message->fields)
        {
            auto index = std::distance(message->fields.data(), &field);
            auto function = std::make_shared<Function>("Get", "return " + field->name + ";\n", ClassName() + "::Type<" + google::protobuf::SimpleItoa(index) + ">&", 0);
            function->Parameter("std::integral_constant<uint32_t, " + google::protobuf::SimpleItoa(index) + ">");
            getters->Add(function);
        }

        classFormatter->Add(getters);
    }

    void MessageGenerator::GenerateNestedMessageAliases()
    {
        if (!message->nestedMessages.empty())
        {
            auto aliases = std::make_shared<Access>("public");

            for (auto& nestedMessage : message->nestedMessages)
                aliases->Add(std::make_shared<Using>(nestedMessage->name + MessageSuffix(), nestedMessage->qualifiedDetailName + MessageSuffix()));

            classFormatter->Add(aliases);
        }
    }

    void MessageGenerator::GenerateEnums()
    {
        if (!message->nestedEnums.empty())
        {
            auto aliases = std::make_shared<Access>("public");

            for (auto& nestedEnum : message->nestedEnums)
                aliases->Add(std::make_shared<Using>(nestedEnum->name, ReferencedEnumPrefix() + nestedEnum->name));

            classFormatter->Add(aliases);
        }
    }

    void MessageGenerator::GenerateNestedMessages(Entities& formatter)
    {
        if (!message->nestedMessages.empty())
        {
            auto nestedMessages = std::make_shared<Namespace>("detail");

            for (auto& nestedMessage : message->nestedMessages)
                MessageGenerator(nestedMessage, ClassName()).Run(*nestedMessages);

            formatter.Add(nestedMessages);
        }
    }

    void MessageGenerator::GenerateFieldDeclarations()
    {
        if (!message->fields.empty())
        {
            auto fields = std::make_shared<Access>("public");

            std::string result;
            StorageTypeVisitor visitor(result);
            for (auto& field : message->fields)
            {
                field->Accept(visitor);
                fields->Add(std::make_shared<DataMember>(field->name, result, "{}"));
            }

            classFormatter->Add(fields);
        }
    }

    void MessageGenerator::GenerateFieldConstants()
    {
        if (!message->fields.empty())
        {
            auto fields = std::make_shared<Access>("public");

            for (auto& field : message->fields)
                fields->Add(std::make_shared<DataMember>(field->constantName, "static const uint32_t", google::protobuf::SimpleItoa(field->number)));

            classFormatter->Add(fields);
        }
    }

    void MessageGenerator::GenerateMaxMessageSize()
    {
        if (message->MaxMessageSize() != infra::none)
        {
            auto fields = std::make_shared<Access>("public");
            fields->Add(std::make_shared<DataMember>("maxMessageSize", "static const uint32_t", google::protobuf::SimpleItoa(*message->MaxMessageSize())));
            classFormatter->Add(fields);
        }
    }

    std::string MessageGenerator::SerializerBody()
    {
        std::ostringstream result;
        {
            google::protobuf::io::OstreamOutputStream stream(&result);
            google::protobuf::io::Printer printer(&stream, '$', nullptr);

            for (auto& field : message->fields)
                printer.Print("SerializeField($type$(), formatter, $name$, $constant$);\n", "type", field->protoType, "name", field->name, "constant", field->constantName);
        }
     
        return result.str();
    }

    std::string MessageGenerator::DeserializerBody()
    {
        std::ostringstream result;
        {
            google::protobuf::io::OstreamOutputStream stream(&result);
            google::protobuf::io::Printer printer(&stream, '$', nullptr);

            printer.Print(R"(while (!parser.Empty())
{
    infra::ProtoParser::Field field = parser.GetField();

)");
            if (!message->fields.empty())
            {
                printer.Print(R"(    switch (field.second)
    {
)");

                printer.Indent(); printer.Indent();

                for (auto& field : message->fields)
                    printer.Print(R"(case $constant$:
    DeserializeField($type$(), parser, field, $name$);
    break;
)"
                        , "constant", field->constantName
                        , "type", field->protoType
                        , "name", field->name);

                printer.Outdent(); printer.Outdent();
        
                printer.Print(R"(        default:
            if (field.first.Is<infra::ProtoLengthDelimited>())
                field.first.Get<infra::ProtoLengthDelimited>().SkipEverything();
            break;
    }
}
)");
            }
            else
                printer.Print(R"(    if (field.first.Is<infra::ProtoLengthDelimited>())
        field.first.Get<infra::ProtoLengthDelimited>().SkipEverything();
}
)");
        }

        return result.str();
    }

    std::string MessageGenerator::CompareEqualBody() const
    {
        std::ostringstream result;
        {
            google::protobuf::io::OstreamOutputStream stream(&result);
            google::protobuf::io::Printer printer(&stream, '$', nullptr);

            printer.Print("return true");

            printer.Indent();
            for (auto& field : message->fields)
                printer.Print("\n&& $name$ == other.$name$", "name", field->name);
            printer.Outdent();

            printer.Print(";\n");
        }

        return result.str();
    }

    std::string MessageGenerator::CompareUnEqualBody() const
    {
        std::ostringstream result;
        {
            google::protobuf::io::OstreamOutputStream stream(&result);
            google::protobuf::io::Printer printer(&stream, '$', nullptr);

            printer.Print("return !(*this == other);\n");
        }

        return result.str();
    }

    std::string MessageGenerator::ClassName() const
    {
        return prefix + message->name;
    }

    std::string MessageGenerator::ReferencedName() const
    {
        return ClassName();
    }

    std::string MessageGenerator::MessageSuffix() const
    {
        return "";
    }

    std::string MessageGenerator::TypeMapName() const
    {
        return "detail::" + ClassName() + "TypeMap";
    }

    std::string MessageGenerator::ReferencedEnumPrefix() const
    {
        return "detail::" + prefix + message->name;
    }

    void MessageReferenceGenerator::GenerateTypeMap(Entities& formatter)
    {
        MessageReferenceTypeMapGenerator typeMapGenerator(message, prefix);
        typeMapGenerator.Run(formatter);
    }

    void MessageReferenceGenerator::GenerateConstructors()
    {
        auto constructors = std::make_shared<Access>("public");
        constructors->Add(std::make_shared<Constructor>(ClassName(), "", Constructor::cDefault));

        if (!message->fields.empty())
        {
            auto constructByMembers = std::make_shared<Constructor>(ClassName(), "", 0);
            std::string result;
            ParameterReferenceTypeVisitor visitor(result);
            for (auto& field : message->fields)
            {
                field->Accept(visitor);
                constructByMembers->Parameter(result + " " + field->name);
                constructByMembers->Initializer(field->name + "(" + field->name + ")");
            }
            constructors->Add(constructByMembers);
        }

        auto constructByProtoParser = std::make_shared<Constructor>(ClassName(), "Deserialize(parser);\n", 0);
        constructByProtoParser->Parameter("infra::ProtoParser& parser");
        constructors->Add(constructByProtoParser);
        classFormatter->Add(constructors);
    }

    void MessageReferenceGenerator::GenerateGetters()
    {
        auto getters = std::make_shared<Access>("public");

        for (auto& field : message->fields)
        {
            auto index = std::distance(message->fields.data(), &field);
            auto function = std::make_shared<Function>("Get", "return " + field->name + ";\n", ClassName() + "::Type<" + google::protobuf::SimpleItoa(index) + ">&", 0);
            function->Parameter("std::integral_constant<uint32_t, " + google::protobuf::SimpleItoa(index) + ">");
            getters->Add(function);
        }

        classFormatter->Add(getters);
    }

    void MessageReferenceGenerator::GenerateNestedMessages(Entities& formatter)
    {
        if (!message->nestedMessages.empty())
        {
            auto nestedMessages = std::make_shared<Namespace>("detail");

            for (auto& nestedMessage : message->nestedMessages)
                MessageReferenceGenerator(nestedMessage, message->name).Run(*nestedMessages);

            formatter.Add(nestedMessages);
        }
    }

    void MessageReferenceGenerator::GenerateFieldDeclarations()
    {
        if (!message->fields.empty())
        {
            auto fields = std::make_shared<Access>("public");

            std::string result;
            ReferenceStorageTypeVisitor visitor(result);
            for (auto& field : message->fields)
            {
                field->Accept(visitor);
                fields->Add(std::make_shared<DataMember>(field->name, result, "{}"));
            }

            classFormatter->Add(fields);
        }
    }

    void MessageReferenceGenerator::GenerateMaxMessageSize()
    {}

    std::string MessageReferenceGenerator::SerializerBody()
    {
        return "std::abort();\n";
    }

    std::string MessageReferenceGenerator::ClassName() const
    {
        return MessageGenerator::ClassName() + "Reference";
    }

    std::string MessageReferenceGenerator::ReferencedName() const
    {
        return "detail::" + ClassName();
    }

    std::string MessageReferenceGenerator::MessageSuffix() const
    {
        return "Reference";
    }

    ServiceGenerator::ServiceGenerator(const std::shared_ptr<const EchoService>& service, Entities& formatter)
        : service(service)
    {
        auto serviceClass = std::make_shared<Class>(service->name);
        serviceClass->Parent("public services::Service");
        serviceFormatter = serviceClass.get();
        formatter.Add(serviceClass);

        auto serviceProxyClass = std::make_shared<Class>(service->name + "Proxy");
        serviceProxyClass->Parent("public services::ServiceProxy");
        serviceProxyFormatter = serviceProxyClass.get();
        formatter.Add(serviceProxyClass);

        GenerateServiceConstructors();
        GenerateServiceProxyConstructors();
        GenerateServiceFunctions();
        GenerateServiceProxyFunctions();
        GenerateFieldConstants();
    }

    void ServiceGenerator::GenerateServiceConstructors()
    {
        auto constructors = std::make_shared<Access>("public");
        auto constructor = std::make_shared<Constructor>(service->name, "", 0);
        constructor->Parameter("services::Echo& echo");
        constructor->Initializer("services::Service(echo, serviceId)");

        constructors->Add(constructor);
        serviceFormatter->Add(constructors);
    }

    void ServiceGenerator::GenerateServiceProxyConstructors()
    {
        auto constructors = std::make_shared<Access>("public");
        auto constructor = std::make_shared<Constructor>(service->name + "Proxy", "", 0);
        constructor->Parameter("services::Echo& echo");
        constructor->Initializer("services::ServiceProxy(echo, serviceId, maxMessageSize)");

        constructors->Add(constructor);
        serviceProxyFormatter->Add(constructors);
    }

    void ServiceGenerator::GenerateServiceFunctions()
    {
        auto functions = std::make_shared<Access>("public");

        for (auto& method: service->methods)
        {
            auto serviceMethod = std::make_shared<Function>(method.name, "", "void", Function::fVirtual | Function::fAbstract);
            if (method.parameter)
            {
                for (auto field: method.parameter->fields)
                {
                    std::string typeName;
                    ParameterTypeVisitor visitor(typeName);
                    field->Accept(visitor);
                    serviceMethod->Parameter(typeName + " " + field->name);
                }
            }
            functions->Add(serviceMethod);
        }

        auto handle = std::make_shared<Function>("Handle", HandleBody(), "void", Function::fVirtual | Function::fOverride);
        handle->Parameter("uint32_t methodId");
        handle->Parameter("infra::ProtoLengthDelimited& contents");
        handle->Parameter("services::EchoErrorPolicy& errorPolicy");
        functions->Add(handle);

        serviceFormatter->Add(functions);
    }

    void ServiceGenerator::GenerateServiceProxyFunctions()
    {
        auto functions = std::make_shared<Access>("public");

        for (auto& method : service->methods)
        {
            auto serviceMethod = std::make_shared<Function>(method.name, ProxyMethodBody(method), "void", 0);
            if (method.parameter)
            {
                for (auto field: method.parameter->fields)
                {
                    std::string typeName;
                    ParameterTypeVisitor visitor(typeName);
                    field->Accept(visitor);
                    serviceMethod->Parameter(typeName + " " + field->name);
                }
            }

            functions->Add(serviceMethod);
        }

        serviceProxyFormatter->Add(functions);
    }

    void ServiceGenerator::GenerateFieldConstants()
    {
        auto fields = std::make_shared<Access>("public");

        fields->Add(std::make_shared<DataMember>("serviceId", "static const uint32_t", google::protobuf::SimpleItoa(service->serviceId)));

        for (auto& method : service->methods)
            fields->Add(std::make_shared<DataMember>("id" + method.name, "static const uint32_t", google::protobuf::SimpleItoa(method.methodId)));

        fields->Add(std::make_shared<DataMember>("maxMessageSize", "static const uint32_t", google::protobuf::SimpleItoa(MaxMessageSize())));

        serviceFormatter->Add(fields);
        serviceProxyFormatter->Add(fields);
    }

    uint32_t ServiceGenerator::MaxMessageSize() const
    {
        uint32_t result = 0;

        for (auto& method : service->methods)
        {
            if (method.parameter && !method.parameter->MaxMessageSize())
                result = std::numeric_limits<uint32_t>::max();
            else if (method.parameter)
                result = std::max<uint32_t>(MaxVarIntSize(service->serviceId) + MaxVarIntSize((method.methodId << 3) | 2) + 10 + *method.parameter->MaxMessageSize(), result);
            else
                result = std::max<uint32_t>(MaxVarIntSize(service->serviceId) + MaxVarIntSize((method.methodId << 3) | 2) + 10, result);
        }

        return result;
    }

    std::string ServiceGenerator::HandleBody() const
    {
        std::ostringstream result;
        {
            google::protobuf::io::OstreamOutputStream stream(&result);
            google::protobuf::io::Printer printer(&stream, '$', nullptr);

            printer.Print(R"(infra::ProtoParser parser(contents.Parser());

switch (methodId)
{
)");

            for (auto& method : service->methods)
            {
                if (method.parameter)
                {
                    printer.Print(R"(    case id$name$:
    {
        $argument$ argument(parser);
        if (!parser.FormatFailed())
            $name$()", "name", method.name, "argument", method.parameter->qualifiedName);

                    for (auto& field: method.parameter->fields)
                    {
                        printer.Print("argument.$field$", "field", field->name);
                        if (&field != &method.parameter->fields.back())
                            printer.Print(", ");
                    }

                    printer.Print(R"();
        break;
    }
)");
                }
                else
                    printer.Print(R"(    case id$name$:
        $name$();
        break;
)", "name", method.name);
            }

            printer.Print(R"(    default:
        errorPolicy.MethodNotFound(ServiceId(), methodId);
        contents.SkipEverything();
)");

            printer.Print("}\n");
        }

        return result.str();
    }

    std::string ServiceGenerator::ProxyMethodBody(const EchoMethod& method) const
    {
        std::ostringstream result;
        {
            google::protobuf::io::OstreamOutputStream stream(&result);
            google::protobuf::io::Printer printer(&stream, '$', nullptr);

            printer.Print(R"(infra::DataOutputStream::WithErrorPolicy stream(Rpc().SendStreamWriter());
infra::ProtoFormatter formatter(stream);
formatter.PutVarInt(serviceId);
{
    infra::ProtoLengthDelimitedFormatter argumentFormatter = formatter.LengthDelimitedFormatter(id$name$);
)", "name", method.name);

            if (method.parameter)
            {
                printer.Print("    $type$(", "type", method.parameter->qualifiedName);

                for (auto& field: method.parameter->fields)
                {
                    std::string typeName;
                    ParameterTypeVisitor visitor(typeName);
                    field->Accept(visitor);
                    printer.Print("$field$", "field", field->name);
                    if (&field != &method.parameter->fields.back())
                        printer.Print(", ");
                }

                printer.Print(R"().Serialize(formatter);
)");
            }

            printer.Print(R"(}
Rpc().Send();
)");
        }

        return result.str();
    }

    EchoGenerator::EchoGenerator(google::protobuf::compiler::GeneratorContext* generatorContext, const std::string& name, const google::protobuf::FileDescriptor* file)
        : stream(generatorContext->Open(name))
        , printer(stream.get(), '$', nullptr)
        , formatter(true)
        , file(file)
    {
        auto includesByHeader = std::make_shared<IncludesByHeader>();
        includesByHeader->Path("infra/util/BoundedString.hpp");
        includesByHeader->Path("infra/util/BoundedVector.hpp");
        includesByHeader->Path("protobuf/echo/Echo.hpp");
        includesByHeader->Path("infra/syntax/ProtoFormatter.hpp");
        includesByHeader->Path("infra/syntax/ProtoParser.hpp");

        EchoRoot root(*file);

        for (auto& dependency : root.GetFile(*file)->dependencies)
            includesByHeader->Path("generated/echo/" + dependency->name + ".pb.hpp");

        formatter.Add(includesByHeader);
        auto includesBySource = std::make_shared<IncludesBySource>();
        includesBySource->Path("generated/echo/" + root.GetFile(*file)->name + ".pb.hpp");
        formatter.Add(includesBySource);

        Entities* currentEntity = &formatter;
        for (auto& package : root.GetFile(*file)->packageParts)
        {
            auto newNamespace = std::make_shared<Namespace>(package);
            auto newEntity = newNamespace.get();
            currentEntity->Add(newNamespace);
            currentEntity = newEntity;
        }

        for (auto& enum_: root.GetFile(*file)->enums)
        {
            enumGenerators.emplace_back(std::make_shared<EnumGenerator>(enum_));
            enumGenerators.back()->Run(*currentEntity);
        }

        for (auto& message : root.GetFile(*file)->messages)
        {
            MessageEnumGenerator messageEnumGenerator(message);
            messageEnumGenerator.Run(*currentEntity);
            messageReferenceGenerators.emplace_back(std::make_shared<MessageReferenceGenerator>(message, ""));
            messageReferenceGenerators.back()->Run(*currentEntity);
            messageGenerators.emplace_back(std::make_shared<MessageGenerator>(message, ""));
            messageGenerators.back()->Run(*currentEntity);
        }

        for (auto& service: root.GetFile(*file)->services)
            serviceGenerators.emplace_back(std::make_shared<ServiceGenerator>(service, *currentEntity));
    }

    void EchoGenerator::GenerateHeader()
    {
        GenerateTopHeaderGuard();
        formatter.PrintHeader(printer);
        GenerateBottomHeaderGuard();
    }

    void EchoGenerator::GenerateSource()
    {
        printer.Print(R"(// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: $filename$

)"
, "filename", file->name());

        formatter.PrintSource(printer, "");
    }

    void EchoGenerator::GenerateTopHeaderGuard()
    {
        std::string filename_identifier = google::protobuf::compiler::cpp::FilenameIdentifier(file->name());

        printer.Print(R"(// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: $filename$

#ifndef echo_$filename_identifier$
#define echo_$filename_identifier$

)"
            , "filename", file->name(), "filename_identifier", filename_identifier);
    }

    void EchoGenerator::GenerateBottomHeaderGuard()
    {
        printer.Print("\n#endif\n");
    }
}
