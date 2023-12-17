add_library(infra.event ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(infra.event PUBLIC
    infra.util
    infra.stream
)

target_sources(infra.event PRIVATE
    AtomicTriggerScheduler.cpp
    AtomicTriggerScheduler.hpp
    ClaimableResource.cpp
    ClaimableResource.hpp
    EventDispatcher.cpp
    EventDispatcher.hpp
    EventDispatcherWithWeakPtr.cpp
    EventDispatcherWithWeakPtr.hpp
    LowPowerEventDispatcher.cpp
    LowPowerEventDispatcher.hpp
    QueueForOneReaderOneIrqWriter.hpp
    SystemStateManager.cpp
    SystemStateManager.hpp
)

add_subdirectory(test)
add_subdirectory(test_helper)
