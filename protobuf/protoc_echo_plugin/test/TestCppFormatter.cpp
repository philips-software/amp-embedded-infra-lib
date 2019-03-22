#include "gmock/gmock.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "infra/util/Optional.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "protobuf/protoc_echo_plugin/CppFormatter.hpp"
#include <sstream>

class EntityMock
    : public application::Entity
{
public:
    MOCK_CONST_METHOD1(PrintHeader, void(google::protobuf::io::Printer& printer));
    MOCK_CONST_METHOD2(PrintSource, void(google::protobuf::io::Printer& printer, const std::string& scope));
};

class CppFormatterTest
    : public testing::Test
{
public:
    CppFormatterTest()
        : stream(infra::inPlace, &stringstream)
        , printer(infra::inPlace, &*stream, '$', nullptr)
    {}

    void ExpectPrinted(const std::string& text)
    {
        printer = infra::none;
        stream = infra::none;
        EXPECT_EQ(text, stringstream.str());
    }

    std::ostringstream stringstream;
    infra::Optional<google::protobuf::io::OstreamOutputStream> stream;
    infra::Optional<google::protobuf::io::Printer> printer;
};

TEST_F(CppFormatterTest, empty_Entities_prints_nothing)
{
    application::Entities entities(false);
    entities.PrintHeader(*printer);
    entities.PrintSource(*printer, "scope::");
    ExpectPrinted("");
}

TEST_F(CppFormatterTest, Entities_prints_header_of_one_entity)
{
    application::Entities entities(false);
    std::unique_ptr<EntityMock> entity(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity, PrintHeader(testing::Ref(*printer)));
    entities.Add(std::move(entity));
    entities.PrintHeader(*printer);
}

TEST_F(CppFormatterTest, Entities_prints_source_of_one_entity)
{
    application::Entities entities(false);
    std::unique_ptr<EntityMock> entity(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity, PrintSource(testing::Ref(*printer), "scope::"));
    entities.Add(std::move(entity));
    entities.PrintSource(*printer, "scope::");
}

TEST_F(CppFormatterTest, multiple_entities_in_header)
{
    application::Entities entities(false);
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintHeader(testing::Ref(*printer))).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer) { printer.Print("a"); }));
    entities.Add(std::move(entity1));
    std::unique_ptr<EntityMock> entity2(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity2, PrintHeader(testing::Ref(*printer))).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer) { printer.Print("b"); }));
    entities.Add(std::move(entity2));
    entities.PrintHeader(*printer);

    ExpectPrinted("ab");
}

TEST_F(CppFormatterTest, multiple_entities_in_header_separated_by_newline)
{
    application::Entities entities(true);
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintHeader(testing::Ref(*printer))).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer) { printer.Print("a"); }));
    entities.Add(std::move(entity1));
    std::unique_ptr<EntityMock> entity2(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity2, PrintHeader(testing::Ref(*printer))).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer) { printer.Print("b"); }));
    entities.Add(std::move(entity2));
    entities.PrintHeader(*printer);

    ExpectPrinted("a\nb");
}

TEST_F(CppFormatterTest, multiple_entities_in_source_separated_with_a_newline)
{
    application::Entities entities(false);
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintSource(testing::Ref(*printer), "scope::")).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer, const std::string& scope) { printer.Print("a"); }));
    entities.Add(std::move(entity1));
    std::unique_ptr<EntityMock> entity2(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity2, PrintSource(testing::Ref(*printer), "scope::")).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer, const std::string& scope) { printer.Print("b"); }));
    entities.Add(std::move(entity2));
    entities.PrintSource(*printer, "scope::");

    ExpectPrinted("a\nb");
}

TEST_F(CppFormatterTest, Class_prints_class_header)
{
    application::Class class_("name");
    class_.PrintHeader(*printer);
    ExpectPrinted(R"(class name
{
};
)");
}

TEST_F(CppFormatterTest, Class_prints_nested_entities_in_header)
{
    application::Class class_("name");
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintHeader(testing::Ref(*printer))).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer) { printer.Print("a\n"); }));
    class_.Add(std::move(entity1));
    class_.PrintHeader(*printer);
    ExpectPrinted(R"(class name
{
    a
};
)");
}

TEST_F(CppFormatterTest, Class_prints_nested_entity_in_source)
{
    application::Class class_("name");
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintSource(testing::Ref(*printer), "scope::name::")).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer, const std::string& scope) { printer.Print("a"); }));
    class_.Add(std::move(entity1));
    class_.PrintSource(*printer, "scope::");
    ExpectPrinted("a");
}

TEST_F(CppFormatterTest, Class_prints_nested_entities_in_source_separated_by_newline)
{
    application::Class class_("name");
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintSource(testing::Ref(*printer), "scope::name::")).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer, const std::string& scope) { printer.Print("a"); }));
    class_.Add(std::move(entity1));
    std::unique_ptr<EntityMock> entity2(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity2, PrintSource(testing::Ref(*printer), "scope::name::")).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer, const std::string& scope) { printer.Print("b"); }));
    class_.Add(std::move(entity2));
    class_.PrintSource(*printer, "scope::");
    ExpectPrinted("a\nb");
}

TEST_F(CppFormatterTest, Class_prints_parent_in_class_header)
{
    application::Class class_("name");
    class_.Parent("parent");
    class_.PrintHeader(*printer);
    ExpectPrinted(R"(class name
    : parent
{
};
)");
}

TEST_F(CppFormatterTest, Class_prints_more_parent_in_class_header)
{
    application::Class class_("name");
    class_.Parent("parent");
    class_.Parent("parent2");
    class_.Parent("parent3");
    class_.PrintHeader(*printer);
    ExpectPrinted(R"(class name
    : parent
    , parent2
    , parent3
{
};
)");
}

TEST_F(CppFormatterTest, Access_prints_level_in_header)
{
    application::Access access("public");
    printer->Indent();
    access.PrintHeader(*printer);
    ExpectPrinted("public:\n");
}

TEST_F(CppFormatterTest, Access_prints_nested_entities_in_header)
{
    application::Access access("public");
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintHeader(testing::Ref(*printer))).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer) { printer.Print("a"); }));
    access.Add(std::move(entity1));
    printer->Indent();
    access.PrintHeader(*printer);
    ExpectPrinted("public:\n    a");
}

TEST_F(CppFormatterTest, Access_prints_nested_entities_in_source)
{
    application::Access access("public");
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintSource(testing::Ref(*printer), "scope")).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer, const std::string& scope) { printer.Print("a"); }));
    access.Add(std::move(entity1));
    access.PrintSource(*printer, "scope");
    ExpectPrinted("a");
}

TEST_F(CppFormatterTest, Namespace_prints_namespace_in_header)
{
    application::Namespace namespace_("n");
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintHeader(testing::Ref(*printer))).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer) { printer.Print("a\n"); }));
    namespace_.Add(std::move(entity1));
    namespace_.PrintHeader(*printer);
    ExpectPrinted(R"(namespace n
{
    a
}
)");
}

TEST_F(CppFormatterTest, Namespace_prints_namespace_in_source)
{
    application::Namespace namespace_("n");
    std::unique_ptr<EntityMock> entity1(std::make_unique<EntityMock>());
    EXPECT_CALL(*entity1, PrintSource(testing::Ref(*printer), "scope")).WillOnce(infra::Lambda([](google::protobuf::io::Printer& printer, const std::string& scope) { printer.Print("a\n"); }));
    namespace_.Add(std::move(entity1));
    namespace_.PrintSource(*printer, "scope");
    ExpectPrinted(R"(namespace n
{
    a
}
)");
}

TEST_F(CppFormatterTest, Function_prints_header)
{
    application::Function function("name", "body\n", "result", 0);
    function.PrintHeader(*printer);
    ExpectPrinted("result name();\n");
}

TEST_F(CppFormatterTest, const_Function_prints_header)
{
    application::Function function("name", "body\n", "result", application::Function::fConst);
    function.PrintHeader(*printer);
    ExpectPrinted("result name() const;\n");
}

TEST_F(CppFormatterTest, virtual_Function_prints_header)
{
    application::Function function("name", "body\n", "result", application::Function::fVirtual);
    function.PrintHeader(*printer);
    ExpectPrinted("virtual result name();\n");
}

TEST_F(CppFormatterTest, abstract_Function_prints_header)
{
    application::Function function("name", "body\n", "result", application::Function::fAbstract);
    function.PrintHeader(*printer);
    ExpectPrinted("result name() = 0;\n");
}

TEST_F(CppFormatterTest, override_Function_prints_header)
{
    application::Function function("name", "body\n", "result", application::Function::fOverride);
    function.PrintHeader(*printer);
    ExpectPrinted("result name() override;\n");
}

TEST_F(CppFormatterTest, Function_with_all_flags_prints_header)
{
    application::Function function("name", "body\n", "result", application::Function::fConst | application::Function::fVirtual | application::Function::fAbstract | application::Function::fOverride);
    function.PrintHeader(*printer);
    ExpectPrinted("virtual result name() const override = 0;\n");
}

TEST_F(CppFormatterTest, Function_prints_source)
{
    application::Function function("name", "body\n", "result", 0);
    function.PrintSource(*printer, "scope::");
    ExpectPrinted(R"(result scope::name()
{
    body
}
)");
}

TEST_F(CppFormatterTest, Function_prints_source_with_empty_body)
{
    application::Function function("name", "", "result", 0);
    function.PrintSource(*printer, "scope::");
    ExpectPrinted(R"(result scope::name()
{}
)");
}

TEST_F(CppFormatterTest, const_Function_prints_source)
{
    application::Function function("name", "body\n", "result", application::Function::fConst);
    function.PrintSource(*printer, "scope::");
    ExpectPrinted(R"(result scope::name() const
{
    body
}
)");
}

TEST_F(CppFormatterTest, Function_with_parameter_prints_header)
{
    application::Function function("name", "body\n", "result", 0);
    function.Parameter("int x");
    function.PrintHeader(*printer);
    ExpectPrinted("result name(int x);\n");
}

TEST_F(CppFormatterTest, Function_with_two_parameters_prints_header)
{
    application::Function function("name", "body\n", "result", 0);
    function.Parameter("int x");
    function.Parameter("char y");
    function.PrintHeader(*printer);
    ExpectPrinted("result name(int x, char y);\n");
}

TEST_F(CppFormatterTest, Function_with_parameter_prints_source)
{
    application::Function function("name", "body\n", "result", 0);
    function.Parameter("int x");
    function.PrintSource(*printer, "scope::");
    ExpectPrinted(R"(result scope::name(int x)
{
    body
}
)");
}

TEST_F(CppFormatterTest, Constructor_prints_header)
{
    application::Constructor constructor("name", "body\n", 0);
    constructor.PrintHeader(*printer);
    ExpectPrinted("name();\n");
}

TEST_F(CppFormatterTest, default_Constructor_prints_header)
{
    application::Constructor constructor("name", "body\n", application::Constructor::cDefault);
    constructor.PrintHeader(*printer);
    ExpectPrinted("name() = default;\n");
}

TEST_F(CppFormatterTest, deleted_Constructor_prints_header)
{
    application::Constructor constructor("name", "body\n", application::Constructor::cDelete);
    constructor.PrintHeader(*printer);
    ExpectPrinted("name() = delete;\n");
}

TEST_F(CppFormatterTest, Constructor_prints_source)
{
    application::Constructor constructor("name", "body\n", 0);
    constructor.PrintSource(*printer, "scope::");
    ExpectPrinted(R"(scope::name()
{
    body
}
)");
}

TEST_F(CppFormatterTest, Constructor_prints_source_with_empty_body)
{
    application::Constructor constructor("name", "", 0);
    constructor.PrintSource(*printer, "scope::");
    ExpectPrinted(R"(scope::name()
{}
)");
}

TEST_F(CppFormatterTest, default_Constructor_prints_no_source)
{
    application::Constructor constructor("name", "body\n", application::Constructor::cDefault);
    constructor.PrintSource(*printer, "scope::");
    ExpectPrinted("");
}

TEST_F(CppFormatterTest, deleted_Constructor_prints_no_source)
{
    application::Constructor constructor("name", "body\n", application::Constructor::cDelete);
    constructor.PrintSource(*printer, "scope::");
    ExpectPrinted("");
}

TEST_F(CppFormatterTest, Constructor_with_parameter_prints_header)
{
    application::Constructor constructor("name", "body\n", 0);
    constructor.Parameter("int x");
    constructor.PrintHeader(*printer);
    ExpectPrinted("name(int x);\n");
}

TEST_F(CppFormatterTest, Constructor_with_two_parameters_prints_header)
{
    application::Constructor constructor("name", "body\n", 0);
    constructor.Parameter("int x");
    constructor.Parameter("char y");
    constructor.PrintHeader(*printer);
    ExpectPrinted("name(int x, char y);\n");
}

TEST_F(CppFormatterTest, Constructor_with_parameter_prints_source)
{
    application::Constructor constructor("name", "body\n", 0);
    constructor.Parameter("int x");
    constructor.PrintSource(*printer, "scope::");
    ExpectPrinted(R"(scope::name(int x)
{
    body
}
)");
}

TEST_F(CppFormatterTest, Constructor_with_initializer_prints_source)
{
    application::Constructor constructor("name", "body\n", 0);
    constructor.Initializer("x(y)");
    constructor.PrintSource(*printer, "scope::");
    ExpectPrinted(R"(scope::name()
    : x(y)
{
    body
}
)");
}

TEST_F(CppFormatterTest, Constructor_with_two_initializers_prints_source)
{
    application::Constructor constructor("name", "body\n", 0);
    constructor.Initializer("x(y)");
    constructor.Initializer("z(a)");
    constructor.PrintSource(*printer, "scope::");
    ExpectPrinted(R"(scope::name()
    : x(y)
    , z(a)
{
    body
}
)");
}

TEST_F(CppFormatterTest, DataMember_prints_header)
{
    application::DataMember member("name", "type");
    member.PrintHeader(*printer);
    ExpectPrinted("type name;\n");
}

TEST_F(CppFormatterTest, initialized_DataMember_prints_header)
{
    application::DataMember initializedMember("name", "type", "initializer");
    initializedMember.PrintHeader(*printer);
    ExpectPrinted("type name = initializer;\n");
}

TEST_F(CppFormatterTest, DataMember_prints_nothing_in_source)
{
    application::DataMember member("name", "type");
    member.PrintSource(*printer, "scope::");
    ExpectPrinted("");
}

TEST_F(CppFormatterTest, IncludesByHeader_prints_header)
{
    application::IncludesByHeader include;
    include.Path("path");
    include.PrintHeader(*printer);
    ExpectPrinted(R"(#include "path"
)");
}

TEST_F(CppFormatterTest, IncludesByHeader_prints_no_source)
{
    application::IncludesByHeader include;;
    include.Path("path");
    include.PrintSource(*printer, "scope");
    ExpectPrinted("");
}

TEST_F(CppFormatterTest, IncludesBySource_prints_no_header)
{
    application::IncludesBySource include;
    include.Path("path");
    include.PrintHeader(*printer);
    ExpectPrinted("");
}

TEST_F(CppFormatterTest, IncludesBySource_prints_source)
{
    application::IncludesBySource include;
    include.Path("path");
    include.PrintSource(*printer, "scope");
    ExpectPrinted(R"(#include "path"
)");
}

TEST_F(CppFormatterTest, ClassForwardDeclaration_prints_header)
{
    application::ClassForwardDeclaration declaration("name");
    declaration.PrintHeader(*printer);
    ExpectPrinted("class name;\n");
}

TEST_F(CppFormatterTest, ClassForwardDeclaration_prints_no_source)
{
    application::ClassForwardDeclaration declaration("name");
    declaration.PrintSource(*printer, "scope");
    ExpectPrinted("");
}
