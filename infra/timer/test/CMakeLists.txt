add_executable(infra.timer_test)
gtest_discover_tests(infra.timer_test AUTO)

target_link_libraries(infra.timer_test
    PUBLIC
    infra.timer
    infra.timer_test_helper

    gtest
    gmock
    gmock_main
)

target_sources(infra.timer_test
    PRIVATE
    TestDerivedTimerService.cpp
    TestRetryPolicy.cpp
    TestScalableDerivedTimerService.cpp
    TestTickOnInterruptTimerService.cpp
    TestTimer.cpp
    TestTimerAlarm.cpp
    TestTimerAlternating.cpp
    TestTimerLimitedRepeating.cpp
    TestTimerLimitedRepeatingWithClosingAction.cpp
)
