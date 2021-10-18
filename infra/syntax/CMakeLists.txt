ccola_component(syntax static_library)

    ccola_sources(
        Asn1.cpp
        Asn1.hpp
        Asn1Formatter.cpp
        Asn1Formatter.hpp
        Json.cpp
        Json.hpp
        JsonFormatter.cpp
        JsonFormatter.hpp
        JsonInputStream.cpp
        JsonInputStream.hpp
        JsonStreamingParser.cpp
        JsonStreamingParser.hpp
        ProtoParser.cpp
        ProtoParser.hpp
        ProtoFormatter.cpp
        ProtoFormatter.hpp
        XmlFormatter.cpp
        XmlFormatter.hpp
      x86:
        JsonObjectNavigator.cpp
        JsonObjectNavigator.hpp
        JsonFileReader.cpp
        JsonFileReader.hpp
    )

    ccola_dependencies(
        stream
    )

ccola_end_component()

ccola_subdirectories(
    test_doubles
    test
)
