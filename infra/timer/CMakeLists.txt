add_library(infra.timer ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(infra.timer PUBLIC
    infra.event
    infra.stream
)

target_sources(infra.timer PRIVATE
    DerivedTimerService.cpp
    DerivedTimerService.hpp
    PartitionedTime.cpp
    PartitionedTime.hpp
    Retry.hpp
    RetryPolicy.cpp
    RetryPolicy.hpp
    ScalableDerivedTimerService.cpp
    ScalableDerivedTimerService.hpp
    TickOnInterruptTimerService.cpp
    TickOnInterruptTimerService.hpp
    Timer.cpp
    Timer.hpp
    TimerAlarm.cpp
    TimerAlarm.hpp
    TimerAlternating.cpp
    TimerAlternating.hpp
    TimerLimitedRepeating.cpp
    TimerLimitedRepeating.hpp
    TimerLimitedRepeatingWithClosingAction.cpp
    TimerLimitedRepeatingWithClosingAction.hpp
    TimerService.cpp
    TimerService.hpp
    TimeStreaming.cpp
    TimeStreaming.hpp
    Waiting.cpp
    Waiting.hpp
)

add_subdirectory(test)
add_subdirectory(test_helper)
