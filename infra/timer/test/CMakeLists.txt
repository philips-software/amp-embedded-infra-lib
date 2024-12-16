add_executable(infra.timer_test)
emil_build_for(infra.timer_test BOOL EMIL_BUILD_TESTS)
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
    TestWaiting.cpp
)
