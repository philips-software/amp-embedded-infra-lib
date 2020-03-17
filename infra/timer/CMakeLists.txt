ccola_component(timer static_library)

    ccola_sources(
        DerivedTimerService.cpp
        DerivedTimerService.hpp
        PartitionedTime.cpp
        PartitionedTime.hpp
        RetryPolicy.cpp
        RetryPolicy.hpp
        ScalableDerivedTimerService.cpp
        ScalableDerivedTimerService.hpp
        TickOnInterruptTimerService.cpp
        TickOnInterruptTimerService.hpp
        Timer.cpp
        Timer.hpp
        TimerAlternating.cpp
        TimerAlternating.hpp
        TimerLimitedRepeating.cpp
        TimerLimitedRepeating.hpp
        TimerLimitedRepeatingWithClosingAction.cpp
        TimerLimitedRepeatingWithClosingAction.hpp
        TimerService.cpp
        TimerService.hpp
        TimerServiceManager.cpp
        TimerServiceManager.hpp
    )

    ccola_dependencies(
        util
        event
    )

ccola_end_component()

ccola_subdirectories(
    test_helper
    test
)
