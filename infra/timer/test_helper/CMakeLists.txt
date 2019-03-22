ccola_component(timer_test_helper x86: static_library other: skip)

    ccola_sources(
        ClockFixture.cpp
        ClockFixture.hpp
        PerfectTimerService.cpp
        PerfectTimerService.hpp
    )

    ccola_dependencies(
        timer
        util_test_helper
    )

ccola_end_component()
