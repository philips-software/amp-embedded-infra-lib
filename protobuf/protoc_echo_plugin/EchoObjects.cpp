#include "generated/proto_cpp/EchoAttributes.pb.h"
#include "google/protobuf/compiler/cpp/cpp_helpers.h"
#include "google/protobuf/stubs/strutil.h"
#include "protobuf/protoc_echo_plugin/EchoObjects.hpp"

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

        std::string QualifiedReferenceName(const google::protobuf::Descriptor& descriptor)
        {
            std::string namespaceString;

            namespaceString = descriptor.file()->package() + "::";
            for (auto containingType = descriptor.containing_type(); containingType != nullptr; containingType = containingType->containing_type())
                namespaceString += containingType->name() + "Reference::";

            return namespaceString + descriptor.name() + "Reference";
        }
    }

    EchoField::EchoField(const google::protobuf::FieldDescriptor& descriptor)
        : name(descriptor.name())
        , number(descriptor.number())
        , constantName(google::protobuf::compiler::cpp::FieldConstantName(&descriptor))
    {}

    std::shared_ptr<EchoField> EchoField::GenerateField(const google::protobuf::FieldDescriptor& fieldDescriptor, EchoRoot& root)
    {
        if (fieldDescriptor.label() != google::protobuf::FieldDescriptor::LABEL_REPEATED)
            switch (fieldDescriptor.type())
            {
                case google::protobuf::FieldDescriptor::TYPE_INT32:
                    return std::make_shared<EchoFieldInt32>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_FIXED32:
                    return std::make_shared<EchoFieldFixed32>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_BOOL:
                    return std::make_shared<EchoFieldBool>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_STRING:
                    return std::make_shared<EchoFieldString>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                    return std::make_shared<EchoFieldMessage>(fieldDescriptor, root);
                case google::protobuf::FieldDescriptor::TYPE_BYTES:
                    return std::make_shared<EchoFieldBytes>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_UINT32:
                    return std::make_shared<EchoFieldUint32>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_ENUM:
                    return std::make_shared<EchoFieldEnum>(fieldDescriptor);
                default:
                    throw UnsupportedFieldType{ fieldDescriptor.name(), fieldDescriptor.type() };
            }
        else
            switch (fieldDescriptor.type())
            {
                case google::protobuf::FieldDescriptor::TYPE_STRING:
                    return std::make_shared<EchoFieldRepeatedString>(fieldDescriptor);
                case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                    return std::make_shared<EchoFieldRepeatedMessage>(fieldDescriptor, root);
                case google::protobuf::FieldDescriptor::TYPE_UINT32:
                    return std::make_shared<EchoFieldRepeatedUint32>(fieldDescriptor);
                default:
                    throw UnsupportedFieldType{ fieldDescriptor.name(), fieldDescriptor.type() };
            }
    }

    EchoEnum::EchoEnum(const google::protobuf::EnumDescriptor& descriptor)
        : name(descriptor.name())
    {
        for (int i = 0; i != descriptor.value_count(); ++i)
            members.push_back(std::make_pair(descriptor.value(i)->name(), descriptor.value(i)->number()));
    }

    EchoMessage::EchoMessage(const google::protobuf::Descriptor& descriptor, EchoRoot& root)
        : descriptor(descriptor)
        , name(descriptor.name())
        , qualifiedName(QualifiedName(descriptor))
        , qualifiedReferenceName(QualifiedReferenceName(descriptor))
    {
        for (int i = 0; i != descriptor.enum_type_count(); ++i)
            nestedEnums.push_back(std::make_shared<EchoEnum>(*descriptor.enum_type(i)));

        for (int i = 0; i != descriptor.nested_type_count(); ++i)
            nestedMessages.push_back(root.AddMessage(*descriptor.nested_type(i)));

        for (int i = 0; i != descriptor.field_count(); ++i)
            fields.emplace_back(EchoField::GenerateField(*descriptor.field(i), root));
    }

    void EchoFieldInt32::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitInt32(*this);
    }

    void EchoFieldFixed32::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitFixed32(*this);
    }

    void EchoFieldBool::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitBool(*this);
    }

    EchoFieldString::EchoFieldString(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField(descriptor)
        , maxStringSize(descriptor.options().GetExtension(string_size))
    {
        if (maxStringSize == 0)
            throw UnspecifiedStringSize{ name };
    }

    void EchoFieldString::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitString(*this);
    }

    EchoFieldMessage::EchoFieldMessage(const google::protobuf::FieldDescriptor& descriptor, EchoRoot& root)
        : EchoField(descriptor)
        , message(root.GetMessage(*descriptor.message_type()))
    {}

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
    }

    void EchoFieldBytes::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitBytes(*this);
    }

    void EchoFieldUint32::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitUint32(*this);
    }

    EchoFieldEnum::EchoFieldEnum(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField(descriptor)
        , typeName(descriptor.enum_type()->name())
        , qualifiedTypeName(QualifiedName(*descriptor.enum_type()))
    {}

    void EchoFieldEnum::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitEnum(*this);
    }

    EchoFieldRepeated::EchoFieldRepeated(const google::protobuf::FieldDescriptor& descriptor)
        : EchoField(descriptor)
        , maxArraySize(descriptor.options().GetExtension(array_size))
    {
        if (maxArraySize == 0)
            throw UnspecifiedArraySize{ name };
    }

    EchoFieldRepeatedString::EchoFieldRepeatedString(const google::protobuf::FieldDescriptor& descriptor)
        : EchoFieldRepeated(descriptor)
        , maxStringSize(descriptor.options().GetExtension(string_size))
    {
        if (maxStringSize == 0)
            throw UnspecifiedStringSize{ name };
    }

    void EchoFieldRepeatedString::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitRepeatedString(*this);
    }

    EchoFieldRepeatedMessage::EchoFieldRepeatedMessage(const google::protobuf::FieldDescriptor& descriptor, EchoRoot& root)
        : EchoFieldRepeated(descriptor)
        , message(root.GetMessage(*descriptor.message_type()))
    {}

    void EchoFieldRepeatedMessage::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitRepeatedMessage(*this);
    }

    void EchoFieldRepeatedUint32::Accept(EchoFieldVisitor& visitor) const
    {
        visitor.VisitRepeatedUint32(*this);
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
