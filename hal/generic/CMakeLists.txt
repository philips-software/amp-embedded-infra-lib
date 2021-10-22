ccola_component(generic x86: static_library other: skip)

    ccola_sources(
        FileSystemGeneric.cpp
        FileSystemGeneric.hpp
        SynchronousRandomDataGeneratorGeneric.cpp
        SynchronousRandomDataGeneratorGeneric.hpp
        TimeKeeperGeneric.cpp
        TimeKeeperGeneric.hpp
        TimerServiceGeneric.cpp
        TimerServiceGeneric.hpp
    )

    ccola_dependencies(
        interfaces
        synchronous_interfaces
    )

    ccola_external_libraries(
    linux:
        pthread
    )

ccola_end_component()
