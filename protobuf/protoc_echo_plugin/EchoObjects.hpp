#ifndef PROTOBUF_ECHO_OBJECTS_HPP
#define PROTOBUF_ECHO_OBJECTS_HPP

#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "infra/util/Optional.hpp"
#include <memory>
#include <vector>

namespace application
{
    uint32_t MaxVarIntSize(uint64_t value);

    class EchoFieldVisitor;
    class EchoRoot;

    class EchoField
    {
    public:
        EchoField(const google::protobuf::FieldDescriptor& descriptor);
        EchoField(const EchoField& other) = delete;
        EchoField& operator=(const EchoField& other) = delete;
        virtual ~EchoField() = default;

        virtual void Accept(EchoFieldVisitor& visitor) const = 0;

        std::string name;
        int number;
        std::string constantName;

        static std::shared_ptr<EchoField> GenerateField(const google::protobuf::FieldDescriptor& fieldDescriptor, EchoRoot& root);
    };

    class EchoEnum
    {
    public:
        EchoEnum(const google::protobuf::EnumDescriptor& descriptor);

        const google::protobuf::EnumDescriptor& descriptor;
        std::string name;
        std::string qualifiedTypeName;
        std::string qualifiedDetailName;
        std::string containedInMessageName;
        std::vector<std::pair<std::string, int>> members;
    };

    class EchoMessage
    {
    public:
        EchoMessage(const google::protobuf::Descriptor& descriptor, EchoRoot& root);

        infra::Optional<uint32_t> MaxMessageSize() const;

        const google::protobuf::Descriptor& descriptor;
        std::string name;
        std::string qualifiedName;
        std::string qualifiedReferenceName;
        std::string qualifiedDetailName;
        std::string qualifiedDetailReferenceName;
        std::vector<std::shared_ptr<EchoField>> fields;
        std::vector<std::shared_ptr<EchoMessage>> nestedMessages;
        std::vector<std::shared_ptr<EchoEnum>> nestedEnums;

    private:
        void ComputeMaxMessageSize();

    private:
        infra::Optional<uint32_t> maxMessageSize;
    };

    class EchoFieldInt32
        : public EchoField
    {
    public:
        using EchoField::EchoField;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldInt64
        : public EchoField
    {
    public:
        using EchoField::EchoField;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldFixed32
        : public EchoField
    {
    public:
        using EchoField::EchoField;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldFixed64
        : public EchoField
    {
    public:
        using EchoField::EchoField;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldSFixed32
        : public EchoField
    {
    public:
        using EchoField::EchoField;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldSFixed64
        : public EchoField
    {
    public:
        using EchoField::EchoField;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldBool
        : public EchoField
    {
    public:
        using EchoField::EchoField;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldString
        : public EchoField
    {
    public:
        EchoFieldString(const google::protobuf::FieldDescriptor& descriptor);

        virtual void Accept(EchoFieldVisitor& visitor) const override;

        uint32_t maxStringSize;
    };

    class EchoFieldStdString
        : public EchoField
    {
    public:
        EchoFieldStdString(const google::protobuf::FieldDescriptor& descriptor);

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldMessage
        : public EchoField
    {
    public:
        EchoFieldMessage(const google::protobuf::FieldDescriptor& descriptor, EchoRoot& root);

        virtual void Accept(EchoFieldVisitor& visitor) const override;

        std::shared_ptr<EchoMessage> message;
        const google::protobuf::FieldDescriptor& descriptor;
    };

    class EchoFieldBytes
        : public EchoField
    {
    public:
        EchoFieldBytes(const google::protobuf::FieldDescriptor& descriptor);

        virtual void Accept(EchoFieldVisitor& visitor) const override;

        uint32_t maxBytesSize;
    };

    class EchoFieldUint32
        : public EchoField
    {
    public:
        using EchoField::EchoField;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldUint64
        : public EchoField
    {
    public:
        using EchoField::EchoField;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoFieldEnum
        : public EchoField
    {
    public:
        EchoFieldEnum(const google::protobuf::FieldDescriptor& descriptor, EchoRoot& root);

        virtual void Accept(EchoFieldVisitor& visitor) const override;

        std::shared_ptr<EchoEnum> type;
    };

    class EchoFieldRepeated
        : public EchoField
    {
    public:
        EchoFieldRepeated(const google::protobuf::FieldDescriptor& descriptor);
      
        uint32_t maxArraySize;
    };

    class EchoFieldRepeatedString
        : public EchoFieldRepeated
    {
    public:
        EchoFieldRepeatedString(const google::protobuf::FieldDescriptor& descriptor);

        virtual void Accept(EchoFieldVisitor& visitor) const override;

        uint32_t maxStringSize;
    };

    class EchoFieldRepeatedMessage
        : public EchoFieldRepeated
    {
    public:
        EchoFieldRepeatedMessage(const google::protobuf::FieldDescriptor& descriptor, EchoRoot& root);

        virtual void Accept(EchoFieldVisitor& visitor) const override;

        std::shared_ptr<EchoMessage> message;
    };

    class EchoFieldRepeatedUint32
        : public EchoFieldRepeated
    {
    public:
        using EchoFieldRepeated::EchoFieldRepeated;

        virtual void Accept(EchoFieldVisitor& visitor) const override;
    };

    class EchoMethod
    {
    public:
        EchoMethod(const google::protobuf::MethodDescriptor& descriptor, EchoRoot& root);

        std::string name;
        uint32_t methodId;
        std::shared_ptr<EchoMessage> parameter;
    };

    class EchoService
    {
    public:
        EchoService(const google::protobuf::ServiceDescriptor& descriptor, EchoRoot& root);

        const google::protobuf::ServiceDescriptor& descriptor;
        std::string name;
        uint32_t serviceId;
        std::vector<EchoMethod> methods;
    };

    class EchoFile
    {
    public:
        EchoFile(const google::protobuf::FileDescriptor& file, EchoRoot& root);

        std::string name;
        std::vector<std::string> packageParts;
        std::vector<std::shared_ptr<EchoEnum>> enums;
        std::vector<std::shared_ptr<EchoMessage>> messages;
        std::vector<std::shared_ptr<EchoService>> services;
        std::vector<std::shared_ptr<EchoFile>> dependencies;
    };

    class EchoRoot
    {
    public:
        EchoRoot() = default;
        EchoRoot(const google::protobuf::FileDescriptor& rootFile);

        void AddDescriptorSet(const google::protobuf::FileDescriptorSet& descriptorSet);

        std::shared_ptr<EchoFile> GetFile(const google::protobuf::FileDescriptor& file);
        std::shared_ptr<EchoMessage> AddMessage(const google::protobuf::Descriptor& descriptor);
        std::shared_ptr<EchoMessage> GetMessage(const google::protobuf::Descriptor& descriptor);
        std::shared_ptr<EchoEnum> AddEnum(const google::protobuf::EnumDescriptor& descriptor);
        std::shared_ptr<EchoEnum> GetEnum(const google::protobuf::EnumDescriptor& descriptor);
        std::shared_ptr<EchoService> AddService(const google::protobuf::ServiceDescriptor& descriptor);

        std::vector<std::shared_ptr<EchoFile>> files;
        std::vector<std::shared_ptr<EchoMessage>> messages;
        std::vector<std::shared_ptr<EchoEnum>> enums;
        std::vector<std::shared_ptr<EchoService>> services;

        google::protobuf::DescriptorPool pool;
    };

    class EchoFieldVisitor
    {
    public:
        EchoFieldVisitor() = default;
        EchoFieldVisitor(const EchoFieldVisitor& other) = delete;
        EchoFieldVisitor& operator=(const EchoFieldVisitor& other) = delete;
        virtual ~EchoFieldVisitor() = default;

        virtual void VisitInt64(const EchoFieldInt64& field) = 0;
        virtual void VisitUint64(const EchoFieldUint64& field) = 0;
        virtual void VisitInt32(const EchoFieldInt32& field) = 0;
        virtual void VisitFixed64(const EchoFieldFixed64& field) = 0;
        virtual void VisitFixed32(const EchoFieldFixed32& field) = 0;
        virtual void VisitBool(const EchoFieldBool& field) = 0;
        virtual void VisitString(const EchoFieldString& field) = 0;
        virtual void VisitStdString(const EchoFieldStdString& field) = 0;
        virtual void VisitMessage(const EchoFieldMessage& field) = 0;
        virtual void VisitBytes(const EchoFieldBytes& field) = 0;
        virtual void VisitUint32(const EchoFieldUint32& field) = 0;
        virtual void VisitEnum(const EchoFieldEnum& field) = 0;
        virtual void VisitSFixed64(const EchoFieldSFixed64& field) = 0;
        virtual void VisitSFixed32(const EchoFieldSFixed32& field) = 0;
        virtual void VisitRepeatedString(const EchoFieldRepeatedString& field) = 0;
        virtual void VisitRepeatedMessage(const EchoFieldRepeatedMessage& field) = 0;
        virtual void VisitRepeatedUint32(const EchoFieldRepeatedUint32& field) = 0;
    };

    struct UnsupportedFieldType
    {
        std::string fieldName;
        google::protobuf::FieldDescriptor::Type type;
    };

    struct UnspecifiedStringSize
    {
        std::string fieldName;
    };

    struct UnspecifiedBytesSize
    {
        std::string fieldName;
    };

    struct UnspecifiedArraySize
    {
        std::string fieldName;
    };

    struct UnspecifiedServiceId
    {
        std::string service;
    };

    struct UnspecifiedMethodId
    {
        std::string service;
        std::string method;
    };

    struct MessageNotFound
    {
        std::string name;
    };

    struct EnumNotFound
    {
        std::string name;
    };
}

#endif
