if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        pugixml
        GIT_REPOSITORY https://github.com/zeux/pugixml
        GIT_TAG ef856bf975f4b1909af1b40ed3d7e28af9263d95 # Unreleased
    )

    FetchContent_MakeAvailable(pugixml)
endif()

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
        CppFormatter.cpp
        CppFormatter.hpp
        JsonObjectNavigator.cpp
        JsonObjectNavigator.hpp
        JsonFileReader.cpp
        JsonFileReader.hpp
        XmlNavigator.cpp
        XmlNavigator.hpp
    )

    target_link_libraries(infra.syntax PUBLIC
        libprotoc
        libprotobuf
        pugixml
    )
endif()

if (EMIL_ENABLE_FUZZING)
    add_subdirectory(fuzz)
endif()

add_subdirectory(test)
add_subdirectory(test_doubles)
