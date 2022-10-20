add_executable(infra.syntax_test)
emil_add_test(infra.syntax_test)

target_link_libraries(infra.syntax_test PUBLIC
    gmock_main
    infra.syntax
    infra.syntax_test_doubles
)

target_sources(infra.syntax_test PRIVATE
    TestAsn1.cpp
    TestAsn1Formatter.cpp
    TestJson.cpp
    TestJsonFileReader.cpp
    TestJsonFormatter.cpp
    TestJsonInputStream.cpp
    TestJsonObjectNavigator.cpp
    TestJsonStreamingParser.cpp
    TestProtoFormatter.cpp
    TestProtoParser.cpp
    TestXmlFormatter.cpp
)

emil_clangformat_target(infra.syntax_test)
