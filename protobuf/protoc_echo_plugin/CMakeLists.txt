ccola_component(protoc_echo_plugin_lib win: static_library other: skip)

    ccola_compile_options(/wd4018 /wd4251 /wd4273 /wd4800)

    ccola_inheritable_definitions(LIBPROTOC_EXPORTS)

    ccola_sources(
        CppFormatter.cpp
        CppFormatter.hpp
        EchoObjects.cpp
        EchoObjects.hpp
        ProtoCEchoPlugin.cpp
        ProtoCEchoPlugin.hpp
    )

    ccola_dependencies(
        libprotobuf
        libprotoc
        echo_attributes
        infra.stream
    )

ccola_end_component()

ccola_component(protoc_echo_plugin win: executable other: skip)

    ccola_compile_options(/wd4018 /wd4251 /wd4273 /wd4800)

    ccola_sources(
        Main.cpp
    )

    ccola_dependencies(
        protoc_echo_plugin_lib
    )

ccola_end_component()
