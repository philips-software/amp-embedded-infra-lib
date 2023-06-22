add_library(infra.timer_test_helper STATIC)
emil_build_for(infra.timer_test_helper BOOL BUILD_TESTING)

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
