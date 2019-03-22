ccola_component(bsd bsd: static_library other: noncompile)

    ccola_sources(
        TimerServiceBsd.cpp
        TimerServiceBsd.hpp
    )

    ccola_dependencies(
        interfaces
        synchronous_interfaces
    )

ccola_end_component()
