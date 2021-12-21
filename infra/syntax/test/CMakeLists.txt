add_executable(infra.syntax_test)
gtest_discover_tests(infra.syntax_test AUTO)

target_link_libraries(infra.syntax_test
    PUBLIC
    infra.syntax
    infra.syntax_test_doubles

    gtest
    gmock
    gmock_main
)

target_sources(infra.syntax_test
    PRIVATE
    TestAsn1.cpp
    TestAsn1Formatter.cpp
    TestJson.cpp
    TestJsonFormatter.cpp
    TestJsonInputStream.cpp
    TestJsonObjectNavigator.cpp
    TestJsonStreamingParser.cpp
    TestProtoFormatter.cpp
    TestProtoParser.cpp
    TestXmlFormatter.cpp
)
