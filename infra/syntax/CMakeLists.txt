add_library(infra.syntax EXCLUDE_FROM_ALL STATIC)
install(TARGETS infra.syntax EXPORT emilInfraTargets)

target_link_libraries(infra.syntax PUBLIC
    infra.stream
)

target_sources(infra.syntax PRIVATE
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
)

if (TARGET_ARCH_X86)
    target_sources(infra.syntax PRIVATE
        JsonObjectNavigator.cpp
        JsonObjectNavigator.hpp
        JsonFileReader.cpp
        JsonFileReader.hpp
    )
endif()

if (EMIL_BUILD_TESTS)
    add_subdirectory(test_doubles)
    add_subdirectory(test)
endif()
