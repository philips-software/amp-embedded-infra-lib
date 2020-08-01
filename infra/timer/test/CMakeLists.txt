ccola_component(timer_test gmock_test)

    ccola_sources(
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

    ccola_dependencies(
        timer
        timer_test_helper
    )

ccola_end_component()
