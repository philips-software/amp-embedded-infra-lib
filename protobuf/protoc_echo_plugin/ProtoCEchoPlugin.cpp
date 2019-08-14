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
        uint32_t MaxVarIntSize(uint32_t value)
        {
            uint32_t result = 1;

            while (value > 127)
            {
                value >>= 7;
                ++result;
            }

            return result;
        }
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
    }

    MessageGenerator::MessageGenerator(const std::shared_ptr<const EchoMessage>& message)
        : message(message)
    {}

    void MessageGenerator::Run(Entities& formatter)
    {
        GenerateClass(formatter);
        GenerateNestedMessageForwardDeclarations();
        GenerateConstructors();
        GenerateFunctions();
        GenerateNestedMessages();
        GenerateFieldDeclarations();
        GenerateFieldConstants();
        GenerateMaxMessageSize();
    }

    void MessageGenerator::GenerateClass(Entities& formatter)
    {
        auto class_ = std::make_shared<Class>(message->name);
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

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                constructor.Parameter("int32_t " + field.name);
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
        constructors->Add(std::make_shared<Constructor>(message->name, "", Constructor::cDefault));

        if (!message->fields.empty())
        {
            auto constructByMembers = std::make_shared<Constructor>(message->name, "", 0);
            GenerateConstructorsVisitor visitor(*constructByMembers);
            for (auto& field : message->fields)
                field->Accept(visitor);
            constructors->Add(constructByMembers);
        }

        auto constructByProtoParser = std::make_shared<Constructor>(message->name, "Deserialize(parser);\n", 0);
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
        compareEqual->Parameter("const " + message->name + "& other");
        functions->Add(compareEqual);

        auto compareUnEqual = std::make_shared<Function>("operator!=", CompareUnEqualBody(), "bool", Function::fConst);
        compareUnEqual->Parameter("const " + message->name + "& other");
        functions->Add(compareUnEqual);

        classFormatter->Add(functions);
    }

    void MessageGenerator::GenerateNestedMessageForwardDeclarations()
    {
        if (!message->nestedMessages.empty())
        {
            auto forwardDeclarations = std::make_shared<Access>("public");

            for (auto& nestedMessage: message->nestedMessages)
                forwardDeclarations->Add(std::make_shared<ClassForwardDeclaration>(nestedMessage->name));

            classFormatter->Add(forwardDeclarations);
        }
    }

    void MessageGenerator::GenerateNestedMessages()
    {
        if (!message->nestedMessages.empty())
        {
            auto nestedMessages = std::make_shared<Access>("public");

            for (auto& nestedMessage : message->nestedMessages)
                MessageGenerator(nestedMessage).Run(*nestedMessages);

            classFormatter->Add(nestedMessages);
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

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int32_t", "0"));
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
        class GenerateMaxMessageSizeVisitor
            : public EchoFieldVisitor
        {
        public:
            GenerateMaxMessageSizeVisitor(std::string& maxMessageSize)
                : maxMessageSize(maxMessageSize)
            {}

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                maxMessageSize += " + " + google::protobuf::SimpleItoa(MaxVarIntSize(std::numeric_limits<uint32_t>::max()) + MaxVarIntSize((field.number << 3) | 2));
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                maxMessageSize += " + " + google::protobuf::SimpleItoa(4 + MaxVarIntSize((field.number << 3) | 2));
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                maxMessageSize += " + " + google::protobuf::SimpleItoa(MaxVarIntSize(1) + MaxVarIntSize((field.number << 3) | 2));
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                maxMessageSize += " + " + google::protobuf::SimpleItoa(field.maxStringSize + MaxVarIntSize(field.maxStringSize) + MaxVarIntSize((field.number << 3) | 2));
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                maxMessageSize += " + " + field.message->qualifiedName + "::maxMessageSize + " + google::protobuf::SimpleItoa(MaxVarIntSize((field.number << 3) | 2));
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                maxMessageSize += " + " + google::protobuf::SimpleItoa(field.maxBytesSize + MaxVarIntSize(field.maxBytesSize) + MaxVarIntSize((field.number << 3) | 2));
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                maxMessageSize += " + " + google::protobuf::SimpleItoa(MaxVarIntSize(std::numeric_limits<uint32_t>::max()) + MaxVarIntSize((field.number << 3) | 2));
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                maxMessageSize += " + " + google::protobuf::SimpleItoa(field.maxArraySize * (field.maxStringSize + MaxVarIntSize(field.maxStringSize) + MaxVarIntSize((field.number << 3) | 2)));
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                maxMessageSize += " + " + google::protobuf::SimpleItoa(field.maxArraySize) + " * " + field.message->qualifiedName + "::maxMessageSize + " +
                    google::protobuf::SimpleItoa(field.maxArraySize * MaxVarIntSize((field.number << 3) | 2));
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                maxMessageSize += " + " + google::protobuf::SimpleItoa(field.maxArraySize * (MaxVarIntSize(std::numeric_limits<uint32_t>::max()) + MaxVarIntSize((field.number << 3) | 2)));
            }

        private:
            std::string& maxMessageSize;
        };

        auto fields = std::make_shared<Access>("public");

        std::string maxMessageSize = "0";
        GenerateMaxMessageSizeVisitor visitor(maxMessageSize);
        for (auto& field : message->fields)
            field->Accept(visitor);

        fields->Add(std::make_shared<DataMember>("maxMessageSize", "static const uint32_t", maxMessageSize));
        
        classFormatter->Add(fields);
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

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                printer.Print("formatter.PutVarIntField($name$, $constant$);\n"
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

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                printer.Print(R"(case $constant$:
    $name$ = static_cast<int32_t>(field.first.Get<uint64_t>());
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                printer.Print(R"(case $constant$:
    $name$ = field.first.Get<uint32_t>();
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                printer.Print(R"(case $constant$:
    $name$ = field.first.Get<uint64_t>() != 0;
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                printer.Print(R"(case $constant$:
    field.first.Get<infra::ProtoLengthDelimited>().GetString($name$);
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                printer.Print(R"(case $constant$:
{
    infra::ProtoParser nestedParser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
    $name$.Deserialize(nestedParser);
    break;
}
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                printer.Print(R"(case $constant$:
    field.first.Get<infra::ProtoLengthDelimited>().GetBytes($name$);
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                printer.Print(R"(case $constant$:
    $name$ = static_cast<uint32_t>(field.first.Get<uint64_t>());
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                printer.Print(R"(case $constant$:
    $name$.emplace_back();
    field.first.Get<infra::ProtoLengthDelimited>().GetString($name$.back());
    break;
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                printer.Print(R"(case $constant$:
{
    infra::ProtoParser parser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
    $name$.emplace_back(parser);
    break;
}
)"
                , "name", field.name
                , "constant", field.constantName);
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                printer.Print(R"(case $constant$:
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

    void MessageReferenceGenerator::GenerateClass(Entities& formatter)
    {
        auto class_ = std::make_shared<Class>(message->name + "Reference");
        classFormatter = class_.get();
        formatter.Add(class_);
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

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                constructor.Parameter("int32_t " + field.name);
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
        constructors->Add(std::make_shared<Constructor>(message->name + "Reference", "", Constructor::cDefault));

        if (!message->fields.empty())
        {
            auto constructByMembers = std::make_shared<Constructor>(message->name + "Reference", "", 0);
            GenerateConstructorsVisitor visitor(*constructByMembers);
            for (auto& field : message->fields)
                field->Accept(visitor);
            constructors->Add(constructByMembers);
        }

        auto constructByProtoParser = std::make_shared<Constructor>(message->name + "Reference", "Deserialize(parser);\n", 0);
        constructByProtoParser->Parameter("infra::ProtoParser& parser");
        constructors->Add(constructByProtoParser);
        classFormatter->Add(constructors);
    }

    void MessageReferenceGenerator::GenerateFunctions()
    {
        auto functions = std::make_shared<Access>("public");

        auto serialize = std::make_shared<Function>("Serialize", SerializerBody(), "void", Function::fConst);
        serialize->Parameter("infra::ProtoFormatter& formatter");
        functions->Add(serialize);

        auto deserialize = std::make_shared<Function>("Deserialize", DeserializerBody(), "void", 0);
        deserialize->Parameter("infra::ProtoParser& parser");
        functions->Add(deserialize);

        auto compareEqual = std::make_shared<Function>("operator==", CompareEqualBody(), "bool", Function::fConst);
        compareEqual->Parameter("const " + message->name + "Reference& other");
        functions->Add(compareEqual);

        auto compareUnEqual = std::make_shared<Function>("operator!=", CompareUnEqualBody(), "bool", Function::fConst);
        compareUnEqual->Parameter("const " + message->name + "Reference& other");
        functions->Add(compareUnEqual);

        classFormatter->Add(functions);
    }

    void MessageReferenceGenerator::GenerateNestedMessageForwardDeclarations()
    {
        if (!message->nestedMessages.empty())
        {
            auto forwardDeclarations = std::make_shared<Access>("public");

            for (auto& nestedMessage : message->nestedMessages)
                forwardDeclarations->Add(std::make_shared<ClassForwardDeclaration>(nestedMessage->name + "Reference"));

            classFormatter->Add(forwardDeclarations);
        }
    }

    void MessageReferenceGenerator::GenerateNestedMessages()
    {
        if (!message->nestedMessages.empty())
        {
            auto nestedMessages = std::make_shared<Access>("public");

            for (auto& nestedMessage : message->nestedMessages)
                MessageReferenceGenerator(nestedMessage).Run(*nestedMessages);

            classFormatter->Add(nestedMessages);
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

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                entities.Add(std::make_shared<DataMember>(field.name, "int32_t", "0"));
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

            virtual void VisitInt32(const EchoFieldInt32& field) override
            {
                printer.Print(R"(case $constant$:
    $name$ = static_cast<int32_t>(field.first.Get<uint64_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitFixed32(const EchoFieldFixed32& field) override
            {
                printer.Print(R"(case $constant$:
    $name$ = field.first.Get<uint32_t>();
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitBool(const EchoFieldBool& field) override
            {
                printer.Print(R"(case $constant$:
    $name$ = field.first.Get<uint64_t>() != 0;
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitString(const EchoFieldString& field) override
            {
                printer.Print(R"(case $constant$:
    field.first.Get<infra::ProtoLengthDelimited>().GetStringReference($name$);
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitMessage(const EchoFieldMessage& field) override
            {
                printer.Print(R"(case $constant$:
{
    infra::ProtoParser nestedParser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
    $name$.Deserialize(nestedParser);
    break;
}
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitBytes(const EchoFieldBytes& field) override
            {
                printer.Print(R"(case $constant$:
    field.first.Get<infra::ProtoLengthDelimited>().GetBytesReference($name$);
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitUint32(const EchoFieldUint32& field) override
            {
                printer.Print(R"(case $constant$:
    $name$ = static_cast<uint32_t>(field.first.Get<uint64_t>());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) override
            {
                printer.Print(R"(case $constant$:
    $name$.emplace_back();
    field.first.Get<infra::ProtoLengthDelimited>().GetStringReference($name$.back());
    break;
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) override
            {
                printer.Print(R"(case $constant$:
{
    infra::ProtoParser parser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
    $name$.emplace_back(parser);
    break;
}
)"
, "name", field.name
, "constant", field.constantName);
            }

            virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) override
            {
                printer.Print(R"(case $constant$:
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
        constructor->Initializer("services::ServiceProxy(echo, serviceId, " + MaxMessageSize() + ")");

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
                serviceMethod->Parameter("const " + method.parameter->qualifiedName + "& argument");
            functions->Add(serviceMethod);
        }

        auto handle = std::make_shared<Function>("Handle", HandleBody(), "void", Function::fVirtual | Function::fOverride);
        handle->Parameter("uint32_t methodId");
        handle->Parameter("infra::ProtoParser& parser");
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
                serviceMethod->Parameter("const " + method.parameter->qualifiedName + "& argument");
            functions->Add(serviceMethod);
        }

        serviceProxyFormatter->Add(functions);
    }

    void ServiceGenerator::GenerateFieldConstants()
    {
        auto fields = std::make_shared<Access>("private");

        fields->Add(std::make_shared<DataMember>("serviceId", "static const uint32_t", google::protobuf::SimpleItoa(service->serviceId)));

        for (auto& method : service->methods)
            fields->Add(std::make_shared<DataMember>("id" + method.name, "static const uint32_t", google::protobuf::SimpleItoa(method.methodId)));

        serviceFormatter->Add(fields);
        serviceProxyFormatter->Add(fields);
    }

    std::string ServiceGenerator::MaxMessageSize() const
    {
        std::string result = "0";
        
        for (auto& method : service->methods)
        {
            if (method.parameter)
                result = "std::max<uint32_t>(" + google::protobuf::SimpleItoa(MaxVarIntSize(service->serviceId) + MaxVarIntSize((method.methodId << 3) | 2)) + " + 10 + " + method.parameter->qualifiedName + "::maxMessageSize, " + result + ")";
            else
                result = "std::max<uint32_t>(" + google::protobuf::SimpleItoa(MaxVarIntSize(service->serviceId) + MaxVarIntSize((method.methodId << 3) | 2)) + " + 10, " + result + ")";
        }

        return result;
    }

    std::string ServiceGenerator::HandleBody() const
    {
        std::ostringstream result;
        {
            google::protobuf::io::OstreamOutputStream stream(&result);
            google::protobuf::io::Printer printer(&stream, '$', nullptr);

            printer.Print("switch (methodId)\n{\n");

            for (auto& method : service->methods)
            {
                if (method.parameter)
                    printer.Print(R"(    case id$name$:
    {
        $argument$ argument(parser);
        $name$(argument);
        break;
    }
)", "name", method.name, "argument", method.parameter->qualifiedName);
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
                printer.Print(R"(    argument.Serialize(formatter);
)");

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

        for (auto& message : root.GetFile(*file)->messages)
        {
            messageReferenceGenerators.emplace_back(std::make_shared<MessageReferenceGenerator>(message));
            messageReferenceGenerators.back()->Run(*currentEntity);
        }

        for (auto& message : root.GetFile(*file)->messages)
        {
            messageGenerators.emplace_back(std::make_shared<MessageGenerator>(message));
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
