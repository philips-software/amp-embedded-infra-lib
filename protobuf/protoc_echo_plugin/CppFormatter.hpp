#ifndef PROTOBUF_CPP_FORMATTER_HPP
#define PROTOBUF_CPP_FORMATTER_HPP

#include "google/protobuf/io/printer.h"
#include <memory>
#include <vector>

namespace application
{
    class Entity
    {
    protected:
        explicit Entity(bool hasHeaderCode = true, bool hasSourceCode = true);
        Entity(const Entity& other) = delete;
        Entity& operator=(const Entity& other) = delete;
        
    public:
        virtual ~Entity() = default;

    public:
        virtual void PrintHeader(google::protobuf::io::Printer& printer) const = 0;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const = 0;

        virtual bool HasHeaderCode() const;
        virtual bool HasSourceCode() const;

    private:
        bool hasHeaderCode;
        bool hasSourceCode;
    };

    class Entities
        : public Entity
    {
    public:
        explicit Entities(bool insertNewlineBetweenEntities, bool hasSourceCode = true);

        void Add(std::shared_ptr<Entity>&& newEntity);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    protected:
        bool EntitiesHaveHeaderCode() const;
        bool EntitiesHaveSourceCode() const;

    private:
        bool insertNewlineBetweenEntities;
        std::vector<std::shared_ptr<Entity>> entities;
    };

    class Class
        : public Entities
    {
    public:
        explicit Class(const std::string& name);

        void Parent(const std::string& parentName);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::string Parents() const;

    private:
        std::string name;
        std::vector<std::string> parents;
    };

    class Access
        : public Entities
    {
    public:
        explicit Access(const std::string& level);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;

        virtual bool HasSourceCode() const override;

    private:
        std::string level;
    };

    class Namespace
        : public Entities
    {
    public:
        explicit Namespace(const std::string& name);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

        virtual bool HasHeaderCode() const override;
        virtual bool HasSourceCode() const override;

    private:
        std::string name;
    };

    class Function
        : public Entity
    {
    public:
        static const uint32_t fConst = 1;
        static const uint32_t fVirtual = 2;
        static const uint32_t fAbstract = 4;
        static const uint32_t fOverride = 8;

        Function(const std::string& name, const std::string& body, const std::string& result, uint32_t flags);

        void Parameter(const std::string& parameter);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::string Parameters() const;

    private:
        std::string name;
        std::string body;
        std::string result;
        uint32_t flags;

        std::vector<std::string> parameters;
    };

    class Constructor
        : public Entity
    {
    public:
        static const uint32_t cDefault = 1;
        static const uint32_t cDelete = 2;

        Constructor(const std::string& name, const std::string& body, uint32_t flags);

        void Parameter(const std::string& parameter);
        void Initializer(const std::string& initializer);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

        virtual bool HasSourceCode() const override;

    private:
        std::string Parameters() const;
        void PrintInitializers(google::protobuf::io::Printer& printer) const;

    private:
        std::string name;
        std::string body;
        uint32_t flags;

        std::vector<std::string> parameters;
        std::vector<std::string> initializers;
    };

    class DataMember
        : public Entity
    {
    public:
        DataMember(const std::string& name, const std::string& type, const std::string& initializer = std::string());

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::string name;
        std::string type;
        std::string initializer;
    };

    class StaticDataMember
        : public Entity
    {
    public:
        StaticDataMember(const std::string& name, const std::string& type, const std::string& initializer);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::string name;
        std::string type;
        std::string initializer;
    };

    class Using
        : public Entity
    {
    public:
        Using(const std::string& name, const std::string& definition);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::string name;
        std::string definition;
    };

    class UsingTemplate
        : public Using
    {
    public:
        using Using::Using;

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

        void TemplateParameter(const std::string& parameter);

    private:
        std::string Parameters() const;

    private:
        std::vector<std::string> parameters;
    };

    class IncludesByHeader
        : public Entity
    {
    public:
        IncludesByHeader();

        void Path(const std::string& path);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::vector<std::string> paths;
    };

    class IncludesBySource
        : public Entity
    {
    public:
        IncludesBySource();

        void Path(const std::string& path);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::vector<std::string> paths;
    };

    class ClassForwardDeclaration
        : public Entity
    {
    public:
        explicit ClassForwardDeclaration(const std::string& name);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::string name;
    };

    class StructTemplateForwardDeclaration
        : public Entity
    {
    public:
        explicit StructTemplateForwardDeclaration(const std::string& name);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

        void TemplateParameter(const std::string& parameter);

    private:
        std::string Parameters() const;

    private:
        std::string name;
        std::vector<std::string> parameters;
    };

    class StructTemplateSpecialization
        : public Entities
    {
    public:
        explicit StructTemplateSpecialization(const std::string& name);

        void TemplateSpecialization(const std::string& specialization);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::string Specializations() const;

    private:
        std::string name;
        std::vector<std::string> specializations;
    };

    class EnumDeclaration
        : public Entity
    {
    public:
        explicit EnumDeclaration(const std::string& name, const std::vector<std::pair<std::string, int>>& members);

        virtual void PrintHeader(google::protobuf::io::Printer& printer) const override;
        virtual void PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const override;

    private:
        std::string name;
        std::vector<std::pair<std::string, int>> members;
    };
}

#endif
