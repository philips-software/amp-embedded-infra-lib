add_library(infra.syntax ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(infra.syntax PUBLIC
    infra.stream
)

target_sources(infra.syntax PRIVATE
    Asn1.cpp
    Asn1.hpp
    Asn1Formatter.cpp
    Asn1Formatter.hpp
    EscapeCharacterHelper.cpp
    EscapeCharacterHelper.hpp
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
)

if (EMIL_HOST_BUILD)
    target_sources(infra.syntax PRIVATE
        JsonObjectNavigator.cpp
        JsonObjectNavigator.hpp
        JsonFileReader.cpp
        JsonFileReader.hpp
    )
endif()

if (EMIL_ENABLE_FUZZING)
    add_subdirectory(fuzz)
endif()

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (BUILD_TESTING)
    add_subdirectory(test_doubles)
endif()
