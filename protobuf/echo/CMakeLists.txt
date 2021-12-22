add_library(protobuf.echo EXCLUDE_FROM_ALL STATIC)

target_link_libraries(protobuf.echo
    PUBLIC

    infra.stream
    infra.syntax
    services.network
)

target_sources(protobuf.echo PRIVATE
    Echo.cpp
    Echo.hpp
    EchoInstantiation.hpp
)

if(BUILD_TESTING)
    add_subdirectory(test_helper)
endif()
