
add_library(infra.event EXCLUDE_FROM_ALL STATIC)

target_link_libraries(infra.event
    PUBLIC
    infra.util
)

target_sources(infra.event PRIVATE
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
