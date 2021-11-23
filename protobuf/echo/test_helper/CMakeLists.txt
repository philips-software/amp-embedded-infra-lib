ccola_component(test_helper static_library)

    ccola_sources(
        EchoSingleLoopback.cpp
        EchoSingleLoopback.hpp
    )

    ccola_dependencies(
        protobuf.echo
    )

ccola_end_component()
