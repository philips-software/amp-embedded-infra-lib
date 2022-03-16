add_library(infra.event ${EMIL_EXCLUDE_FROM_ALL} STATIC)
install(TARGETS infra.event EXPORT emilInfraTargets)

target_link_libraries(infra.event
    PUBLIC
        infra.util
    PRIVATE
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

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
add_subdirectory(test_helper)
