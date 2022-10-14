add_executable(infra.timer_test)
emil_add_test(infra.timer_test)

target_link_libraries(infra.timer_test PUBLIC
    gmock_main
    infra.timer
    infra.timer_test_helper
)

target_sources(infra.timer_test PRIVATE
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

emil_clangformat_target(infra.timer_test)
