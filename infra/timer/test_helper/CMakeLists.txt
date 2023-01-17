add_library(infra.timer_test_helper ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(infra.timer_test_helper PRIVATE
    infra.timer
    infra.util_test_helper
)

target_sources(infra.timer_test_helper PRIVATE
    ClockFixture.cpp
    ClockFixture.hpp
    PerfectTimerService.cpp
    PerfectTimerService.hpp
)
