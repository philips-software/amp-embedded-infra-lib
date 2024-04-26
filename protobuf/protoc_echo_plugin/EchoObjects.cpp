#include "protobuf/protoc_echo_plugin/EchoObjects.hpp"
#include "generated/EchoAttributes.pb.h"
#include "google/protobuf/compiler/cpp/helpers.h"
#include "google/protobuf/stubs/strutil.h"
#include "infra/syntax/ProtoFormatter.hpp"

namespace application
{
    namespace
    {
        std::string QualifiedName(const google::protobuf::Descriptor& descriptor)
        {
            std::string namespaceString;

            namespaceString = descriptor.file()->package() + "::";
            for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
                namespaceString += containingType->name() + "::";

            return namespaceString + descriptor.name();
        }

        std::string QualifiedName(const google::protobuf::EnumDescriptor& descriptor)
        {
            std::string namespaceString;

            namespaceString = descriptor.file()->package() + "::";
            for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
                namespaceString += containingType->name() + "::";

            return namespaceString + descriptor.name();
        }

        std::string QualifiedDetailName(const google::protobuf::EnumDescriptor& descriptor)
        {
            std::string namespaceString;

            namespaceString = descriptor.file()->package() + "::";

            if (descriptor.containing_type() != nullptr)
            {
                namespaceString += "detail::";

                for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
                    namespaceString += containingType->name();
            }

            return namespaceString + descriptor.name();
        }

        std::string QualifiedReferenceName(const google::protobuf::Descriptor& descriptor)
        {
            std::string namespaceString;

            namespaceString = descriptor.file()->package() + "::";
            for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
                namespaceString += containingType->name() + "Reference::";

            return namespaceString + descriptor.name() + "Reference";
        }

        std::string QualifiedDetailName(const google::protobuf::Descriptor& descriptor)
        {
            std::string namespaceString;

            namespaceString = descriptor.file()->package() + "::";

            if (descriptor.containing_type() != nullptr)
            {
                namespaceString += "detail::";

                for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
                    namespaceString += containingType->name();
            }

            return namespaceString + descriptor.name();
        }

        std::string QualifiedDetailReferenceName(const google::protobuf::Descriptor& descriptor)
        {
            std::string namespaceString;

            namespaceString = descriptor.file()->package() + "::";

            if (descriptor.containing_type() != nullptr)
            {
                namespaceString += "detail::";

                for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
                    namespaceString += containingType->name();
            }

            return namespaceString + descriptor.name() + "Reference";
        }
    }

    EchoField::EchoField(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("", descriptor)
    {}

    EchoField::EchoField(const std::string& protoType, const google::protobuf::FieldDescriptor& descriptor)
        : protoType(protoType)
        , protoReferenceType(protoType)
        , name(descriptor.name())
        , number(descriptor.number())
        , constantName(google::protobuf::compiler::cpp::FieldConstantName(&descriptor))
    {}

    std::shared_ptr<EchoField> EchoField::GenerateField(const google::protobuf::FieldDescriptor& fieldDescriptor, EchoRoot& root)
    {
        if (fieldDescriptor.label() != google::protobuf::FieldDescriptor::LABEL_REPEATED)
            switch (fieldDescriptor.type())
            {
                case google::protobuf::FieldDescriptor::TYPE_INT64:
                    return std::make_shared<EchoFieldInt64>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_UINT64:
                    return std::make_shared<EchoFieldUint64>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_INT32:
                    return std::make_shared<EchoFieldInt32>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                    return std::make_shared<EchoFieldFixed64>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_FIXED32:
                    return std::make_shared<EchoFieldFixed32>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_BOOL:
                    return std::make_shared<EchoFieldBool>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_STRING:
                    if (fieldDescriptor.options().GetExtension(string_size) != 0)
                        return std::make_shared<EchoFieldString>(fieldDescriptor);
                    else
                        return std::make_shared<EchoFieldUnboundedString>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                    return std::make_shared<EchoFieldMessage>(fieldDescriptor, root);
                case google::protobuf::FieldDescriptor::TYPE_BYTES:
                    if (fieldDescriptor.options().GetExtension(bytes_size) != 0)
                        return std::make_shared<EchoFieldBytes>(fieldDescriptor);
                    else
                        return std::make_shared<EchoFieldUnboundedBytes>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_UINT32:
                    return std::make_shared<EchoFieldUint32>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_ENUM:
                    return std::make_shared<EchoFieldEnum>(fieldDescriptor, root);
                case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
                    return std::make_shared<EchoFieldSFixed64>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
                    return std::make_shared<EchoFieldSFixed32>(fieldDescriptor);
                default:
                    throw UnsupportedFieldType{ fieldDescriptor.name(), fieldDescriptor.type() };
            }
        else if (fieldDescriptor.options().GetExtension(array_size) != 0)
            switch (fieldDescriptor.type())
            {
                case google::protobuf::FieldDescriptor::TYPE_INT64:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldInt64>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_UINT64:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldUint64>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_INT32:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldInt32>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldFixed64>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_FIXED32:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldFixed32>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_BOOL:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldBool>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_STRING:
                    if (fieldDescriptor.options().GetExtension(string_size) != 0)
                        return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldString>(fieldDescriptor));
                    else
                        return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldUnboundedString>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldMessage>(fieldDescriptor, root));
                case google::protobuf::FieldDescriptor::TYPE_BYTES:
                    if (fieldDescriptor.options().GetExtension(bytes_size) != 0)
                        return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldBytes>(fieldDescriptor));
                    else
                        return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldUnboundedBytes>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_UINT32:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldUint32>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_ENUM:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldEnum>(fieldDescriptor, root));
                case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldSFixed64>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
                    return std::make_shared<EchoFieldRepeated>(fieldDescriptor, std::make_shared<EchoFieldSFixed32>(fieldDescriptor));
                default:
                    throw UnsupportedFieldType{ fieldDescriptor.name(), fieldDescriptor.type() };
            }
        else
            switch (fieldDescriptor.type())
            {
                case google::protobuf::FieldDescriptor::TYPE_INT64:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldInt64>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_UINT64:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldUint64>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_INT32:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldInt32>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldFixed64>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_FIXED32:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldFixed32>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_BOOL:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldBool>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_STRING:
                    if (fieldDescriptor.options().GetExtension(string_size) != 0)
                        return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldString>(fieldDescriptor));
                    else
                        return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldUnboundedString>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldMessage>(fieldDescriptor, root));
                case google::protobuf::FieldDescriptor::TYPE_BYTES:
                    if (fieldDescriptor.options().GetExtension(bytes_size) != 0)
                        return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldBytes>(fieldDescriptor));
                    else
                        return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldUnboundedBytes>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_UINT32:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldUint32>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_ENUM:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldEnum>(fieldDescriptor, root));
                case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldSFixed64>(fieldDescriptor));
                case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
                    return std::make_shared<EchoFieldUnboundedRepeated>(fieldDescriptor, std::make_shared<EchoFieldSFixed32>(fieldDescriptor));
                default:
                    throw UnsupportedFieldType{ fieldDescriptor.name(), fieldDescriptor.type() };
            }
    }

    EchoEnum::EchoEnum(const google::protobuf::EnumDescriptor& descriptor)
        : descriptor(descriptor)
        , name(descriptor.name())
        , qualifiedTypeName(QualifiedName(descriptor))
        , qualifiedDetailName(QualifiedDetailName(descriptor))
    {
        for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
            containedInMessageName += containingType->name();

        for (int i = 0; i != descriptor.value_count(); ++i)
            members.push_back(std::make_pair(descriptor.value(i)->name(), descriptor.value(i)->number()));
    }

    EchoMessage::EchoMessage(const google::protobuf::Descriptor& descriptor, EchoRoot& root)
        : descriptor(descriptor)
        , name(descriptor.name())
        , qualifiedName(QualifiedName(descriptor))
        , qualifiedReferenceName(QualifiedReferenceName(descriptor))
        , qualifiedDetailName(QualifiedDetailName(descriptor))
        , qualifiedDetailReferenceName(QualifiedDetailReferenceName(descriptor))
    {
        for (int i = 0; i != descriptor.enum_type_count(); ++i)
            nestedEnums.push_back(root.AddEnum(*descriptor.enum_type(i)));

        for (int i = 0; i != descriptor.nested_type_count(); ++i)
            nestedMessages.push_back(root.AddMessage(*descriptor.nested_type(i)));

        for (int i = 0; i != descriptor.field_count(); ++i)
            fields.emplace_back(EchoField::GenerateField(*descriptor.field(i), root));

        ComputeMaxMessageSize();
    }

    std::optional<uint32_t> EchoMessage::MaxMessageSize() const
    {
        return maxMessageSize;
    }

    void EchoMessage::ComputeMaxMessageSize()
    {
        struct NoMaxMessageSize
        {};

        class GenerateMaxMessageSizeVisitor
            : public EchoFieldVisitor
        {
        public:
            explicit GenerateMaxMessageSizeVisitor(uint32_t& maxMessageSize)
                : maxMessageSize(maxMessageSize)
            {}

            void VisitInt64(const EchoFieldInt64& field) override
            {
                maxMessageSize += infra::MaxVarIntSize(std::numeric_limits<uint64_t>::max()) + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitUint64(const EchoFieldUint64& field) override
            {
                maxMessageSize += infra::MaxVarIntSize(std::numeric_limits<uint64_t>::max()) + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitInt32(const EchoFieldInt32& field) override
            {
                maxMessageSize += infra::MaxVarIntSize(std::numeric_limits<uint32_t>::max()) + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitFixed64(const EchoFieldFixed64& field) override
            {
                maxMessageSize += 8 + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitFixed32(const EchoFieldFixed32& field) override
            {
                maxMessageSize += 4 + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitBool(const EchoFieldBool& field) override
            {
                maxMessageSize += infra::MaxVarIntSize(1) + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitString(const EchoFieldString& field) override
            {
                maxMessageSize += field.maxStringSize + infra::MaxVarIntSize(field.maxStringSize) + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitUnboundedString(const EchoFieldUnboundedString& field) override
            {
                throw NoMaxMessageSize();
            }

            void VisitMessage(const EchoFieldMessage& field) override
            {
                uint32_t fieldMaxMessageSize = 0;
                GenerateMaxMessageSizeVisitor visitor(fieldMaxMessageSize);
                for (auto& field : field.message->fields)
                    field->Accept(visitor);

                maxMessageSize += fieldMaxMessageSize + infra::MaxVarIntSize(fieldMaxMessageSize) + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitBytes(const EchoFieldBytes& field) override
            {
                maxMessageSize += field.maxBytesSize + infra::MaxVarIntSize(field.maxBytesSize) + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitUnboundedBytes(const EchoFieldUnboundedBytes& field) override
            {
                throw NoMaxMessageSize();
            }

            void VisitUint32(const EchoFieldUint32& field) override
            {
                maxMessageSize += infra::MaxVarIntSize(std::numeric_limits<uint32_t>::max()) + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitEnum(const EchoFieldEnum& field) override
            {
                maxMessageSize += infra::MaxVarIntSize(std::numeric_limits<uint32_t>::max()) + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                maxMessageSize += 8 + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                maxMessageSize += 4 + infra::MaxVarIntSize((field.number << 3) | 2);
            }

            void VisitRepeated(const EchoFieldRepeated& field) override
            {
                uint32_t max = 0;
                GenerateMaxMessageSizeVisitor visitor(max);
                field.type->Accept(visitor);
                maxMessageSize += field.maxArraySize * max;
            }

            void VisitUnboundedRepeated(const EchoFieldUnboundedRepeated& field) override
            {
                throw NoMaxMessageSize();
            }

        private:
            uint32_t& maxMessageSize;
        };

        try
        {
            uint32_t max = 0;
            GenerateMaxMessageSizeVisitor visitor(max);
            for (auto& field : fields)
                field->Accept(visitor);

            maxMessageSize = max;
        }
        catch (NoMaxMessageSize&) //NOSONAR
        {}
    }

    EchoFieldInt32::EchoFieldInt32(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoInt32", descriptor)
    {}

    void EchoFieldInt32::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitInt32(*this);
    }

    EchoFieldInt64::EchoFieldInt64(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoInt64", descriptor)
    {}

    void EchoFieldInt64::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitInt64(*this);
    }

    EchoFieldFixed32::EchoFieldFixed32(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoFixed32", descriptor)
    {}

    void EchoFieldFixed32::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitFixed32(*this);
    }

    EchoFieldFixed64::EchoFieldFixed64(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoFixed64", descriptor)
    {}

    void EchoFieldFixed64::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitFixed64(*this);
    }

    EchoFieldBool::EchoFieldBool(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoBool", descriptor)
    {}

    void EchoFieldBool::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitBool(*this);
    }

    EchoFieldString::EchoFieldString(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField(descriptor)
        , maxStringSize(descriptor.options().GetExtension(string_size))
    {
        assert(maxStringSize != 0);
        protoReferenceType = protoType = "services::ProtoString<" + google::protobuf::SimpleItoa(maxStringSize) + ">";
    }

    void EchoFieldString::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitString(*this);
    }

    EchoFieldUnboundedString::EchoFieldUnboundedString(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoUnboundedString", descriptor)
    {}

    void EchoFieldUnboundedString::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitUnboundedString(*this);
    }

    EchoFieldMessage::EchoFieldMessage(const google::protobuf::FieldDescriptor& descriptor, EchoRoot& root)
        : EchoField(descriptor)
        , message(root.GetMessage(*descriptor.message_type()))
        , descriptor(descriptor)
    {
        protoType = "services::ProtoMessage<" + message->qualifiedDetailName + ">";
        protoReferenceType = "services::ProtoMessage<" + message->qualifiedDetailReferenceName + ">";
    }

    void EchoFieldMessage::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitMessage(*this);
    }

    EchoFieldBytes::EchoFieldBytes(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField(descriptor)
        , maxBytesSize(descriptor.options().GetExtension(bytes_size))
    {
        if (maxBytesSize == 0)
            throw UnspecifiedBytesSize{ name };

        protoReferenceType = protoType = "services::ProtoBytes<" + google::protobuf::SimpleItoa(maxBytesSize) + ">";
    }

    void EchoFieldBytes::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitBytes(*this);
    }

    EchoFieldUnboundedBytes::EchoFieldUnboundedBytes(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField(descriptor)
    {
        protoReferenceType = protoType = "services::ProtoUnboundedBytes";
    }

    void EchoFieldUnboundedBytes::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitUnboundedBytes(*this);
    }

    EchoFieldUint32::EchoFieldUint32(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoUInt32", descriptor)
    {}

    void EchoFieldUint32::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitUint32(*this);
    }

    EchoFieldUint64::EchoFieldUint64(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoUInt64", descriptor)
    {}

    void EchoFieldUint64::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitUint64(*this);
    }

    EchoFieldEnum::EchoFieldEnum(const google::protobuf::FieldDescriptor& descriptor, EchoRoot& root)
        : EchoField(descriptor)
        , type(root.GetEnum(*descriptor.enum_type()))
    {
        protoReferenceType = protoType = "services::ProtoEnum<" + type->qualifiedDetailName + ">";
    }

    void EchoFieldEnum::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitEnum(*this);
    }

    EchoFieldSFixed32::EchoFieldSFixed32(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoSFixed32", descriptor)
    {}

    void EchoFieldSFixed32::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitSFixed32(*this);
    }

    EchoFieldSFixed64::EchoFieldSFixed64(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField("services::ProtoSFixed64", descriptor)
    {}

    void EchoFieldSFixed64::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitSFixed64(*this);
    }

    EchoFieldRepeated::EchoFieldRepeated(const google::protobuf::FieldDescriptor& descriptor, const std::shared_ptr<EchoField>& type)
        : EchoField(descriptor)
        , maxArraySize(descriptor.options().GetExtension(array_size))
        , type(type)
    {
        if (maxArraySize == 0)
            throw UnspecifiedArraySize{ name };

        protoType = "services::ProtoRepeated<" + google::protobuf::SimpleItoa(maxArraySize) + ", " + type->protoType + ">";
        protoReferenceType = "services::ProtoRepeated<" + google::protobuf::SimpleItoa(maxArraySize) + ", " + type->protoReferenceType + ">";
    }

    void EchoFieldRepeated::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitRepeated(*this);
    }

    EchoFieldUnboundedRepeated::EchoFieldUnboundedRepeated(const google::protobuf::FieldDescriptor& descriptor, const std::shared_ptr<EchoField>& type)
        : EchoField(descriptor)
        , type(type)
    {
        protoType = "services::ProtoUnboundedRepeated<" + type->protoType + ">";
        protoReferenceType = "services::ProtoUnboundedRepeated<" + type->protoReferenceType + ">";
    }

    void EchoFieldUnboundedRepeated::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitUnboundedRepeated(*this);
    }

    EchoMethod::EchoMethod(const google::protobuf::MethodDescriptor& descriptor, EchoRoot& root)
        : name(descriptor.name())
        , methodId(descriptor.options().GetExtension(method_id))
    {
        if (methodId == 0)
            throw UnspecifiedMethodId{ "", name };

        if (descriptor.input_type()->full_name() != Nothing::descriptor()->full_name())
            parameter = root.GetMessage(*descriptor.input_type());
    }

    EchoService::EchoService(const google::protobuf::ServiceDescriptor& descriptor, EchoRoot& root)
        : descriptor(descriptor)
        , name(descriptor.name())
        , serviceId(descriptor.options().GetExtension(service_id))
    {
        if (serviceId == 0)
            throw UnspecifiedServiceId{ name };

        try
        {
            for (int i = 0; i != descriptor.method_count(); ++i)
                methods.emplace_back(*descriptor.method(i), root);
        }
        catch (UnspecifiedMethodId& exception)
        {
            throw UnspecifiedMethodId{ name, exception.method };
        }
    }

    EchoFile::EchoFile(const google::protobuf::FileDescriptor& file, EchoRoot& root)
    {
        name = google::protobuf::compiler::cpp::StripProto(file.name());
        packageParts = google::protobuf::Split(file.package(), ".", true);

        for (int i = 0; i != file.dependency_count(); ++i)
            if (file.dependency(i)->name() != "EchoAttributes.proto")
                dependencies.push_back(root.GetFile(*file.dependency(i)));

        for (int i = 0; i != file.enum_type_count(); ++i)
            enums.push_back(root.AddEnum(*file.enum_type(i)));

        for (int i = 0; i != file.message_type_count(); ++i)
            messages.push_back(root.AddMessage(*file.message_type(i)));

        for (int i = 0; i != file.service_count(); ++i)
            services.push_back(root.AddService(*file.service(i)));
    }

    EchoRoot::EchoRoot(const google::protobuf::FileDescriptor& rootFile)
    {
        GetFile(rootFile);
    }

    void EchoRoot::AddDescriptorSet(const google::protobuf::FileDescriptorSet& descriptorSet)
    {
        std::vector<const google::protobuf::FileDescriptor*> fileDescriptors;

        for (int i = 0; i != descriptorSet.file_size(); ++i)
        {
            auto file = pool.BuildFile(descriptorSet.file(i));
            assert(file != NULL);
            fileDescriptors.push_back(file);
        }

        GetFile(*fileDescriptors.back());
    }

    std::shared_ptr<EchoFile> EchoRoot::GetFile(const google::protobuf::FileDescriptor& fileDescriptor)
    {
        for (auto file : files)
            if (file->name == google::protobuf::compiler::cpp::StripProto(fileDescriptor.name()))
                return file;

        auto result = std::make_shared<EchoFile>(fileDescriptor, *this);
        files.push_back(result);
        return result;
    }

    std::shared_ptr<EchoMessage> EchoRoot::AddMessage(const google::protobuf::Descriptor& descriptor)
    {
        for (auto message : messages)
            if (&message->descriptor == &descriptor)
                std::abort();

        auto result = std::make_shared<EchoMessage>(descriptor, *this);
        messages.push_back(result);
        return result;
    }

    std::shared_ptr<EchoMessage> EchoRoot::GetMessage(const google::protobuf::Descriptor& descriptor)
    {
        for (auto message : messages)
            if (&message->descriptor == &descriptor)
                return message;

        throw MessageNotFound{ descriptor.name() };
    }

    std::shared_ptr<EchoEnum> EchoRoot::AddEnum(const google::protobuf::EnumDescriptor& descriptor)
    {
        for (auto enum_ : enums)
            if (&enum_->descriptor == &descriptor)
                std::abort();

        auto result = std::make_shared<EchoEnum>(descriptor);
        enums.push_back(result);
        return result;
    }

    std::shared_ptr<EchoEnum> EchoRoot::GetEnum(const google::protobuf::EnumDescriptor& descriptor)
    {
        for (auto enum_ : enums)
            if (&enum_->descriptor == &descriptor)
                return enum_;

        throw EnumNotFound{ descriptor.name() };
    }

    std::shared_ptr<EchoService> EchoRoot::AddService(const google::protobuf::ServiceDescriptor& descriptor)
    {
        for (auto service : services)
            if (&service->descriptor == &descriptor)
                std::abort();

        auto result = std::make_shared<EchoService>(descriptor, *this);
        services.push_back(result);
        return result;
    }
}
