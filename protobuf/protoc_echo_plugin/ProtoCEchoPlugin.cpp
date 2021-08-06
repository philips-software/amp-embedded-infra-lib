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
        class TypeNameVisitor
            : public EchoFieldVisitor
        {
        public:
            TypeNameVisitor(std::string& result)
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
                result = "infra::BoundedConstString";
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                result = "std::string";
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                result = "const " + field.message->qualifiedName + "&";
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                result = "const infra::BoundedVector<uint8_t>&";
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                result = "uint32_t";
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                result = field.type->qualifiedTypeName;
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                result = "int64_t";
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                result = "int32_t";
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                result = "const infra::BoundedVector<infra::BoundedString::WithStorage<" + google::protobuf::SimpleItoa(field.maxStringSize) + ">>&";
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                result = "const infra::BoundedVector<" + field.message->qualifiedName + ">&";
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                result = "const infra::BoundedVector<uint32_t>&";
            }

        private:
            std::string& result;
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
        class GenerateTypeMapEntryVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateTypeMapEntryVisitor(Entities& entities, const std::string& messageSuffix)
                : entities(entities)
                , messageSuffix(messageSuffix)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoInt64"));
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoUInt64"));
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoInt32"));
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoFixed64"));
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoFixed32"));
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoBool"));
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoString<" + google::protobuf::SimpleItoa(field.maxStringSize) + ">"));
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoStdString"));
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoEnum<" + field.type->containedInMessageName + field.type->name + ">"));
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoSFixed64"));
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoSFixed32"));
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoMessage<" + field.message->qualifiedDetailName + messageSuffix + ">"));
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoBytes<" + google::protobuf::SimpleItoa(field.maxBytesSize) + ">"));
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoUInt32"));
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoRepeated<" + google::protobuf::SimpleItoa(field.maxArraySize) + ", services::ProtoString<" + google::protobuf::SimpleItoa(field.maxStringSize) + ">>"));
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoRepeated<" + google::protobuf::SimpleItoa(field.maxArraySize) + ", services::ProtoMessage<" + field.message->qualifiedDetailName + messageSuffix + ">>"));
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                entities.Add(std::make_shared<Using>("ProtoType", "services::ProtoRepeated<" + google::protobuf::SimpleItoa(field.maxArraySize) + ", services::ProtoUInt32>"));
            }

        private:
            Entities& entities;
            std::string messageSuffix;
        };

        auto typeMapNamespace = std::make_shared<Namespace>("detail");

        auto typeMapDeclaration = std::make_shared<StructTemplateForwardDeclaration>(MessageName() + "TypeMap");
        typeMapDeclaration->TemplateParameter("std::size_t fieldIndex");
        typeMapNamespace->Add(typeMapDeclaration);

        for (auto& field : message->fields)
        {
            auto typeMapSpecialization = std::make_shared<StructTemplateSpecialization>(MessageName() + "TypeMap");
            typeMapSpecialization->TemplateSpecialization(google::protobuf::SimpleItoa(std::distance(message->fields.data(), &field)));
            GenerateTypeMapEntryVisitor visitor(*typeMapSpecialization, MessageSuffix());
            field->Accept(visitor);
            AddTypeMapType(*field, *typeMapSpecialization, MessageSuffix());
            typeMapNamespace->Add(typeMapSpecialization);
        }

        formatter.Add(typeMapNamespace);
    }

    void MessageTypeMapGenerator::AddTypeMapType(EchoField& field, Entities& entities, const std::string& messageSuffix)
    {
        class GenerateTypeMapEntryVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateTypeMapEntryVisitor(Entities& entities, const std::string& messageSuffix)
                : entities(entities)
                , messageSuffix(messageSuffix)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "int64_t"));
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "uint64_t"));
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "int32_t"));
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "uint64_t"));
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "uint32_t"));
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "bool"));
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::BoundedString::WithStorage<" + google::protobuf::SimpleItoa(field.maxStringSize) + ">"));
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "std::string"));
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                entities.Add(std::make_shared<Using>("Type", field.type->containedInMessageName + field.type->name));
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "int64_t"));
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "int32_t"));
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                entities.Add(std::make_shared<Using>("Type", field.message->qualifiedDetailName + messageSuffix));
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::BoundedVector<uint8_t>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxBytesSize) + ">"));
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "uint32_t"));
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::BoundedVector<infra::BoundedString::WithStorage<" + google::protobuf::SimpleItoa(field.maxStringSize) + ">>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::BoundedVector<" + field.message->qualifiedDetailName + messageSuffix + ">::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::BoundedVector<uint32_t>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

        private:
            Entities& entities;
            std::string messageSuffix;
        };

        GenerateTypeMapEntryVisitor visitor(entities, messageSuffix);
        field.Accept(visitor);
    }

    std::string MessageTypeMapGenerator::MessageName() const
    {
        return prefix + message->name + MessageSuffix();
    }

    std::string MessageTypeMapGenerator::MessageSuffix() const
    {
        return "";
    }

    void MessageReferenceTypeMapGenerator::AddTypeMapType(EchoField& field, Entities& entities, const std::string& messageSuffix)
    {
        class GenerateTypeMapEntryVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateTypeMapEntryVisitor(Entities& entities)
                : entities(entities)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "int64_t"));
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "uint64_t"));
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "int32_t"));
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "uint64_t"));
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "uint32_t"));
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "bool"));
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::BoundedConstString"));
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "std::string"));
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                entities.Add(std::make_shared<Using>("Type", field.type->containedInMessageName + field.type->name));
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "int64_t"));
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "int32_t"));
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                entities.Add(std::make_shared<Using>("Type", field.message->qualifiedDetailName + "Reference"));
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::ConstByteRange"));
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "uint32_t"));
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::BoundedVector<infra::BoundedConstString>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::BoundedVector<" + field.message->qualifiedDetailName + "Reference>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                entities.Add(std::make_shared<Using>("Type", "infra::BoundedVector<uint32_t>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

        private:
            Entities& entities;
        };

        GenerateTypeMapEntryVisitor visitor(entities);
        field.Accept(visitor);
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
        class GenerateConstructorsVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateConstructorsVisitor(Constructor& constructor)
                : constructor(constructor)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                constructor.Parameter("int64_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                constructor.Parameter("uint64_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                constructor.Parameter("int32_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                constructor.Parameter("uint64_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                constructor.Parameter("uint32_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                constructor.Parameter("bool " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                constructor.Parameter("infra::BoundedConstString " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                constructor.Parameter("std::string " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                constructor.Parameter("const " + field.message->qualifiedName + "& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                constructor.Parameter("const infra::BoundedVector<uint8_t>& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                constructor.Parameter("uint32_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                constructor.Parameter(field.type->name + " " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                constructor.Parameter("int64_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                constructor.Parameter("int32_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                constructor.Parameter("const infra::BoundedVector<infra::BoundedString::WithStorage<" + google::protobuf::SimpleItoa(field.maxStringSize) + ">>& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                constructor.Parameter("const infra::BoundedVector<" + field.message->qualifiedName + ">& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                constructor.Parameter("const infra::BoundedVector<uint32_t>& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

        private:
            Constructor& constructor;
        };

        auto constructors = std::make_shared<Access>("public");
        constructors->Add(std::make_shared<Constructor>(ClassName(), "", Constructor::cDefault));

        if (!message->fields.empty())
        {
            auto constructByMembers = std::make_shared<Constructor>(ClassName(), "", 0);
            GenerateConstructorsVisitor visitor(*constructByMembers);
            for (auto& field : message->fields)
                field->Accept(visitor);
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
        class GenerateFieldDeclarationVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateFieldDeclarationVisitor(Entities& entities)
                : entities(entities)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int64_t", "0"));
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "uint64_t", "0"));
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int32_t", "0"));
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "uint64_t", "0"));
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "uint32_t", "0"));
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "bool", "false"));
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "infra::BoundedString::WithStorage<" + google::protobuf::SimpleItoa(field.maxStringSize) + ">"));
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "std::string"));
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, field.type->name, "{}"));
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int64_t", "0"));
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int32_t", "0"));
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, field.message->qualifiedName));
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "infra::BoundedVector<uint8_t>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxBytesSize) + ">"));
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "uint32_t", "0"));
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name
                    , "infra::BoundedVector<infra::BoundedString::WithStorage<" + google::protobuf::SimpleItoa(field.maxStringSize) + ">>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name
                    , "infra::BoundedVector<" + field.message->qualifiedName + ">::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name
                    , "infra::BoundedVector<uint32_t>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

        private:
            Entities& entities;
        };

        if (!message->fields.empty())
        {
            auto fields = std::make_shared<Access>("public");

            GenerateFieldDeclarationVisitor visitor(*fields);
            for (auto& field : message->fields)
                field->Accept(visitor);

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
        class GenerateSerializerBodyVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateSerializerBodyVisitor(google::protobuf::io::Printer& printer)
                : printer(printer)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                printer.Print("formatter.PutVarIntField($name$, $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                printer.Print("formatter.PutVarIntField($name$, $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                printer.Print("formatter.PutVarIntField($name$, $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                printer.Print("formatter.PutFixed64Field($name$, $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                printer.Print("formatter.PutFixed32Field($name$, $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                printer.Print("formatter.PutVarIntField($name$, $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                printer.Print("formatter.PutStringField($name$, $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                printer.Print("formatter.PutStringField($name$, $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                printer.Print("formatter.PutVarIntField(static_cast<uint64_t>($name$), $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                printer.Print("formatter.PutFixed64Field(static_cast<uint64_t>($name$), $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                printer.Print("formatter.PutFixed32Field(static_cast<uint32_t>($name$), $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                printer.Print(R"({
    infra::ProtoLengthDelimitedFormatter nestedMessage(formatter, $constant$);
    $name$.Serialize(formatter);
}
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                printer.Print("formatter.PutBytesField(infra::MakeRange($name$), $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                printer.Print("formatter.PutVarIntField($name$, $constant$);\n"
                    , "name", field.name
                    , "constant", field.constantName);
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                printer.Print(R"(for (auto& subField : $name$)
    formatter.PutStringField(subField, $constant$);
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                printer.Print(R"(if (!$name$.empty())
{
    for (auto& subField : $name$)
    {
        infra::ProtoLengthDelimitedFormatter subFormatter = formatter.LengthDelimitedFormatter($constant$);
        subField.Serialize(formatter);
    }
}
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                printer.Print(R"(for (auto& subField : $name$)
    formatter.PutVarIntField(subField, $constant$);
)"
                , "name", field.name
                , "constant", field.constantName);
            }

        private:
            google::protobuf::io::Printer& printer;
        };

        std::ostringstream result;
        {
            google::protobuf::io::OstreamOutputStream stream(&result);
            google::protobuf::io::Printer printer(&stream, '$', nullptr);

            GenerateSerializerBodyVisitor visitor(printer);
            for (auto& field : message->fields)
                field->Accept(visitor);
        }
     
        return result.str();
    }

    std::string MessageGenerator::DeserializerBody()
    {
        class GenerateDeserializerBodyVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateDeserializerBodyVisitor(google::protobuf::io::Printer& printer)
                : printer(printer)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<int64_t>(field.first.Get<uint64_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = field.first.Get<uint64_t>();
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<int32_t>(field.first.Get<uint64_t>());
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = field.first.Get<uint64_t>();
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint32_t>());
    if (field.first.Is<uint32_t>())
        $name$ = field.first.Get<uint32_t>();
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = field.first.Get<uint64_t>() != 0;
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
        field.first.Get<infra::ProtoLengthDelimited>().GetString($name$);
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
        $name$ = field.first.Get<infra::ProtoLengthDelimited>().GetStdString();
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }
            
            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<$type$>(field.first.Get<uint64_t>());
    break;
)"
                , "name", field.name
                , "type", field.type->name
                , "constant", field.constantName);
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<int64_t>(field.first.Get<uint64_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint32_t>());
    if (field.first.Is<uint32_t>())
        $name$ = static_cast<int32_t>(field.first.Get<uint32_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
    {
        infra::ProtoParser nestedParser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
        $name$.Deserialize(nestedParser);
    }
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
        field.first.Get<infra::ProtoLengthDelimited>().GetBytes($name$);
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<uint32_t>(field.first.Get<uint64_t>());
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
    {
        $name$.emplace_back();
        field.first.Get<infra::ProtoLengthDelimited>().GetString($name$.back());
    }
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
    {
        infra::ProtoParser parser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
        $name$.emplace_back(parser);
    }
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$.push_back(static_cast<uint32_t>(field.first.Get<uint64_t>()));
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

        private:
            google::protobuf::io::Printer& printer;
        };

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
                GenerateDeserializerBodyVisitor visitor(printer);
                for (auto& field : message->fields)
                    field->Accept(visitor);
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
        class GenerateConstructorsVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateConstructorsVisitor(Constructor& constructor)
                : constructor(constructor)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                constructor.Parameter("int64_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                constructor.Parameter("uint64_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                constructor.Parameter("int32_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                constructor.Parameter("uint64_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                constructor.Parameter("uint32_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                constructor.Parameter("bool " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                constructor.Parameter("infra::BoundedConstString " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                constructor.Parameter("const std::string& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                constructor.Parameter(field.type->name + " " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                constructor.Parameter("int64_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                constructor.Parameter("int32_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                constructor.Parameter("const " + field.message->qualifiedReferenceName + "& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                constructor.Parameter("infra::ConstByteRange " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                constructor.Parameter("uint32_t " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                constructor.Parameter("const infra::BoundedVector<infra::BoundedConstString>& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                constructor.Parameter("const infra::BoundedVector<" + field.message->qualifiedReferenceName + ">& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                constructor.Parameter("const infra::BoundedVector<uint32_t>& " + field.name);
                constructor.Initializer(field.name + "(" + field.name + ")");
            }

        private:
            Constructor& constructor;
        };

        auto constructors = std::make_shared<Access>("public");
        constructors->Add(std::make_shared<Constructor>(ClassName(), "", Constructor::cDefault));

        if (!message->fields.empty())
        {
            auto constructByMembers = std::make_shared<Constructor>(ClassName(), "", 0);
            GenerateConstructorsVisitor visitor(*constructByMembers);
            for (auto& field : message->fields)
                field->Accept(visitor);
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
        class GenerateFieldDeclarationVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateFieldDeclarationVisitor(Entities& entities)
                : entities(entities)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int64_t", "0"));
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "uint64_t", "0"));
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int32_t", "0"));
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "uint64_t", "0"));
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "uint32_t", "0"));
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "bool", "false"));
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "infra::BoundedConstString"));
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "std::string"));
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, field.message->qualifiedReferenceName));
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "infra::ConstByteRange"));
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "uint32_t", "0"));
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, field.type->name, "{}"));
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int64_t", "0"));
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int32_t", "0"));
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name
                    , "infra::BoundedVector<infra::BoundedConstString>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name
                    , "infra::BoundedVector<" + field.message->qualifiedReferenceName + ">::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name
                    , "infra::BoundedVector<uint32_t>::WithMaxSize<" + google::protobuf::SimpleItoa(field.maxArraySize) + ">"));
            }

        private:
            Entities& entities;
        };

        if (!message->fields.empty())
        {
            auto fields = std::make_shared<Access>("public");

            GenerateFieldDeclarationVisitor visitor(*fields);
            for (auto& field : message->fields)
                field->Accept(visitor);

            classFormatter->Add(fields);
        }
    }

    void MessageReferenceGenerator::GenerateMaxMessageSize()
    {}

    std::string MessageReferenceGenerator::SerializerBody()
    {
        return "std::abort();\n";
    }

    std::string MessageReferenceGenerator::DeserializerBody()
    {
        class GenerateDeserializerBodyVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateDeserializerBodyVisitor(google::protobuf::io::Printer& printer)
                : printer(printer)
            {}

            virtual void VisitInt64(const EchoFieldInt64& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<int64_t>(field.first.Get<uint64_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitUint64(const EchoFieldUint64& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = field.first.Get<uint64_t>();
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<int32_t>(field.first.Get<uint64_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitFixed64(const EchoFieldFixed64& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = field.first.Get<uint64_t>();
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint32_t>());
    if (field.first.Is<uint32_t>())
        $name$ = field.first.Get<uint32_t>();
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = field.first.Get<uint64_t>() != 0;
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
        field.first.Get<infra::ProtoLengthDelimited>().GetStringReference($name$);
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitStdString(const EchoFieldStdString& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
        $name$ = field.first.Get<infra::ProtoLengthDelimited>().GetStdString();
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
    {
        infra::ProtoParser nestedParser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
        $name$.Deserialize(nestedParser);
    }
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
        field.first.Get<infra::ProtoLengthDelimited>().GetBytesReference($name$);
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<uint32_t>(field.first.Get<uint64_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitEnum(const EchoFieldEnum& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<$type$>(field.first.Get<uint64_t>());
    break;
)"
                , "name", field.name
                , "type", field.type->name
                , "constant", field.constantName);
            }

            virtual void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$ = static_cast<int64_t>(field.first.Get<uint64_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint32_t>());
    if (field.first.Is<uint32_t>())
        $name$ = static_cast<int32_t>(field.first.Get<uint32_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
    {
        $name$.emplace_back();
        field.first.Get<infra::ProtoLengthDelimited>().GetStringReference($name$.back());
    }
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<infra::ProtoLengthDelimited>());
    if (field.first.Is<infra::ProtoLengthDelimited>())
    {
        infra::ProtoParser parser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
        $name$.emplace_back(parser);
    }
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                printer.Print(R"(case $constant$:
    parser.ReportResult(field.first.Is<uint64_t>());
    if (field.first.Is<uint64_t>())
        $name$.push_back(static_cast<uint32_t>(field.first.Get<uint64_t>()));
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

        private:
            google::protobuf::io::Printer& printer;
        };

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
                GenerateDeserializerBodyVisitor visitor(printer);
                for (auto& field : message->fields)
                    field->Accept(visitor);
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
                    TypeNameVisitor visitor(typeName);
                    field->Accept(visitor);
                    serviceMethod->Parameter(typeName + " " + field->name);
                }
            }
            functions->Add(serviceMethod);
        }

        auto handle = std::make_shared<Function>("Handle", HandleBody(), "void", Function::fVirtual | Function::fOverride);
        handle->Parameter("uint32_t methodId");
        handle->Parameter("infra::ProtoLengthDelimited& contents");
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
                    TypeNameVisitor visitor(typeName);
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
    {
        $name$();
        break;
    }
)", "name", method.name);
            }

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
                    TypeNameVisitor visitor(typeName);
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
