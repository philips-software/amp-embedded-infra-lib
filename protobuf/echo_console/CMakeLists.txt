ccola_component(echo_console win: executable other: skip)

    ccola_compile_options(/wd4018 /wd4251 /wd4273 /wd4800)

    ccola_sources(
        Console.cpp
        Console.hpp
        Main.cpp
    )

    ccola_dependencies(
        protoc_echo_plugin_lib
        echo
        hal.generic
        hal.windows
        services.network_win
        arg_parser.args
    )

ccola_end_component()
