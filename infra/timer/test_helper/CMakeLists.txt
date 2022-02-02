add_library(infra.timer_test_helper EXCLUDE_FROM_ALL STATIC)

target_link_libraries(infra.timer_test_helper PUBLIC
    gmock
    infra.timer
    infra.util_test_helper
)

target_sources(infra.timer_test_helper PRIVATE
    ClockFixture.cpp
    ClockFixture.hpp
    PerfectTimerService.cpp
    PerfectTimerService.hpp
)
