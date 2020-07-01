ccola_component(syntax_test gmock_test)

    ccola_sources(
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

    ccola_dependencies(
        syntax
    )

ccola_end_component()
