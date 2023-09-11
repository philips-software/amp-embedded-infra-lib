if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        pugixml
        GIT_REPOSITORY https://github.com/zeux/pugixml
        GIT_TAG a0e064336317c9347a91224112af9933598714e9 # v1.13
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
        protobuf::libprotoc
        pugixml
    )
endif()

if (EMIL_ENABLE_FUZZING)
    add_subdirectory(fuzz)
endif()

add_subdirectory(test)
add_subdirectory(test_doubles)
