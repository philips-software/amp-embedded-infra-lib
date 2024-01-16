add_library(infra.event ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(infra.event PUBLIC
    infra.util
    infra.stream
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

if (EMIL_HOST_BUILD)
    target_sources(infra.event PRIVATE
        EventDispatcherThreadAware.cpp
        EventDispatcherThreadAware.hpp
    )
endif()

add_subdirectory(test)
add_subdirectory(test_helper)
