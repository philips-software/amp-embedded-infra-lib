#include "protobuf/protoc_echo_plugin/CppFormatter.hpp"
#include <functional>

namespace application
{
    template<class C>
    void ForEach(const C& elements, std::function<void(const typename C::value_type&)> each, std::function<void()> between)
    {
        if (!elements.empty())
        {
            each(elements.front());

            for (auto element = std::next(elements.begin()); element != elements.end(); ++element)
            {
                between();
                each(*element);
            }
        }
    }

    template<class C>
    C Filter(const C& elements, std::function<bool(const typename C::value_type&)> filter)
    {
        C result;

        for (auto& element : elements)
            if (filter(element))
                result.push_back(element);

        return result;
    }

    Entity::Entity(bool hasHeaderCode, bool hasSourceCode)
        : hasHeaderCode(hasHeaderCode)
        , hasSourceCode(hasSourceCode)
    {}

    bool Entity::HasHeaderCode() const
    {
        return hasHeaderCode;
    }

    bool Entity::HasSourceCode() const
    {
        return hasSourceCode;
    }

    Entities::Entities(bool insertNewlineBetweenEntities)
        : insertNewlineBetweenEntities(insertNewlineBetweenEntities)
    {}

    void Entities::Add(std::shared_ptr<Entity>&& newEntity)
    {
        entities.push_back(std::move(newEntity));
    }

    void Entities::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        if (insertNewlineBetweenEntities)
            ForEach(Filter(entities, [](const std::shared_ptr<Entity>& entity) { return entity->HasHeaderCode(); }), [&printer](const std::shared_ptr<Entity>& entity) { entity->PrintHeader(printer); }, [&printer]() { printer.Print("\n"); });
        else
            for (auto& entity : entities)
                entity->PrintHeader(printer);
    }

    void Entities::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {
        ForEach(Filter(entities, [](const std::shared_ptr<Entity>& entity) { return entity->HasSourceCode(); }), [&printer, &scope](const std::shared_ptr<Entity>& entity) { entity->PrintSource(printer, scope); }, [&printer]() { printer.Print("\n"); });
    }

    bool Entities::EntitiesHaveHeaderCode() const
    {
        for (auto& entity : entities)
            if (entity->HasHeaderCode())
                return true;

        return false;
    }

    bool Entities::EntitiesHaveSourceCode() const
    {
        for (auto& entity : entities)
            if (entity->HasSourceCode())
                return true;

        return false;
    }

    Class::Class(const std::string& name)
        : Entities(true)
        , name(name)
    {}

    void Class::Parent(const std::string& parentName)
    {
        parents.push_back(parentName);
    }

    void Class::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        printer.Print(R"(class $name$
)", "name", name);

        if (!parents.empty())
            printer.Print("    : $parents$\n", "parents", Parents());

        printer.Print("{\n");
        printer.Indent();
        Entities::PrintHeader(printer);
        printer.Outdent();
        printer.Print(R"(};
)");
    }

    void Class::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {
        Entities::PrintSource(printer, scope + name + "::");
    }

    std::string Class::Parents() const
    {
        std::string res;
        ForEach(parents, [&res](const std::string& parent) { res += parent; }, [&res]() { res += "\n    , "; });
        return res;
    }

    Access::Access(const std::string& level)
        : Entities(false)
        , level(level)
    {}

    void Access::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        printer.Outdent();
        printer.Print("$level$:\n", "level", level);
        printer.Indent();
        Entities::PrintHeader(printer);
    }

    bool Access::HasSourceCode() const
    {
        return EntitiesHaveSourceCode();
    }

    Namespace::Namespace(const std::string& name)
        : Entities(true)
        , name(name)
    {}

    void Namespace::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        printer.Print("namespace $name$\n{\n", "name", name);
        printer.Indent();
        Entities::PrintHeader(printer);
        printer.Outdent();
        printer.Print("}\n");
    }

    void Namespace::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {
        printer.Print("namespace $name$\n{\n", "name", name);
        printer.Indent();
        Entities::PrintSource(printer, scope);
        printer.Outdent();
        printer.Print("}\n");
    }

    bool Namespace::HasHeaderCode() const
    {
        return EntitiesHaveHeaderCode();
    }

    bool Namespace::HasSourceCode() const
    {
        return EntitiesHaveSourceCode();
    }

    Function::Function(const std::string& name, const std::string& body, const std::string& result, uint32_t flags)
        : name(name)
        , body(body)
        , result(result)
        , flags(flags)
    {}

    void Function::Parameter(const std::string& parameter)
    {
        parameters.push_back(parameter);
    }

    void Function::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        printer.Print("$virtual$$result$ $name$($parameters$)$const$$override$$abstract$;\n"
            , "result", result
            , "name", name
            , "parameters", Parameters()
            , "const", (flags & fConst) != 0 ? " const" : ""
            , "virtual", (flags & fVirtual) != 0 ? "virtual " : ""
            , "abstract", (flags & fAbstract) != 0 ? " = 0" : ""
            , "override", (flags & fOverride) != 0 ? " override" : "");
    }

    void Function::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {
        printer.Print("$result$ $scope$$name$($parameters$)$const$\n"
            , "result", result
            , "scope", scope
            , "name", name
            , "parameters", Parameters()
            , "const", (flags & fConst) != 0 ? " const" : "");

        if (body.empty())
            printer.Print("{}\n");
        else
        {
            printer.Print("{\n");
            printer.Indent();
            printer.Print(body.c_str());
            printer.Outdent();
            printer.Print("}\n");
        }
    }

    std::string Function::Parameters() const
    {
        std::string res;
        ForEach(parameters, [&res](const std::string& parameter) { res += parameter; }, [&res]() { res += ", "; });
        return res;
    }

    Constructor::Constructor(const std::string& name, const std::string& body, uint32_t flags)
        : name(name)
        , body(body)
        , flags(flags)
    {}

    void Constructor::Parameter(const std::string& parameter)
    {
        parameters.push_back(parameter);
    }

    void Constructor::Initializer(const std::string& initializer)
    {
        initializers.push_back(initializer);
    }

    void Constructor::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        printer.Print("$name$($parameters$)$default$$delete$;\n"
            , "name", name
            , "parameters", Parameters()
            , "default", (flags & cDefault) != 0 ? " = default" : ""
            , "delete", (flags & cDelete) != 0 ? " = delete" : "");
    }

    void Constructor::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {
        if ((flags & cDefault) == 0 && (flags & cDelete) == 0)
        {
            printer.Print(R"($scope$$name$($parameters$)
)"
                , "scope", scope
                , "name", name
                , "parameters", Parameters());

            printer.Indent();
            PrintInitializers(printer);
            printer.Outdent();
        
            if (body.empty())
                printer.Print("{}\n");
            else
            {
                printer.Print("{\n");
                printer.Indent();
                printer.Print(body.c_str());
                printer.Outdent();
                printer.Print("}\n");
            }
        };
    }

    bool Constructor::HasSourceCode() const
    {
        return flags == 0;
    }

    std::string Constructor::Parameters() const
    {
        std::string result;
        ForEach(parameters, [&result](const std::string& parameter) { result += parameter; }, [&result]() { result += ", "; });
        return result;
    }

    void Constructor::PrintInitializers(google::protobuf::io::Printer& printer) const
    {
        std::string separator = ": ";
        
        for (auto& initializer : initializers)
        {
            printer.Print("$separator$$initializer$\n", "separator", separator, "initializer", initializer);
            separator = ", ";
        }
    }

    DataMember::DataMember(const std::string& name, const std::string& type, const std::string& initializer)
        : Entity(true, false)
        , name(name)
        , type(type)
        , initializer(initializer)
    {}

    void DataMember::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        if (initializer.empty())
            printer.Print("$type$ $name$;\n", "type", type, "name", name);
        else
            printer.Print("$type$ $name$ = $initializer$;\n", "type", type, "name", name, "initializer", initializer);
    }

    void DataMember::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {}

    IncludesByHeader::IncludesByHeader()
        : Entity(true, false)
    {}

    void IncludesByHeader::Path(const std::string& path)
    {
        paths.push_back(path);
    }

    void IncludesByHeader::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        for (auto& path : paths)
            printer.Print(R"(#include "$path$"
)", "path", path);
    }

    void IncludesByHeader::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {}

    IncludesBySource::IncludesBySource()
        : Entity(false, true)
    {}

    void IncludesBySource::Path(const std::string& path)
    {
        paths.push_back(path);
    }

    void IncludesBySource::PrintHeader(google::protobuf::io::Printer& printer) const
    {}

    void IncludesBySource::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {
        for (auto& path : paths)
            printer.Print(R"(#include "$path$"
)", "path", path);
    }

    ClassForwardDeclaration::ClassForwardDeclaration(const std::string& name)
        : Entity(true, false)
        , name(name)
    {}

    void ClassForwardDeclaration::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        printer.Print("class $name$;\n", "name", name);
    }

    void ClassForwardDeclaration::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {}

    EnumDeclaration::EnumDeclaration(const std::string& name, const std::vector<std::pair<std::string, int>>& members)
        : Entity(true, false)
        , name(name)
        , members(members)
    {}

    void EnumDeclaration::PrintHeader(google::protobuf::io::Printer& printer) const
    {
        printer.Print("enum class $name$\n{\n", "name", name);
        printer.Indent();

        for (auto& member : members)
        {
            printer.Print("$name$ = $initializer$", "name", member.first, "initializer", std::to_string(member.second));

            if (&member != &members.back())
                printer.Print(",");
            printer.Print("\n");
        }

        printer.Outdent();
        printer.Print("};\n", "name", name);
    }

    void EnumDeclaration::PrintSource(google::protobuf::io::Printer& printer, const std::string& scope) const
    {
    }
}
