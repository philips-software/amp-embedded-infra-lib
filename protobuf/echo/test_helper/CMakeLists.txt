ccola_component(test_helper static_library)

    ccola_sources(
        EchoOnStreamsHelper.cpp
        EchoOnStreamsHelper.hpp
    )

    ccola_dependencies(
        protobuf.echo
    )

ccola_end_component()
