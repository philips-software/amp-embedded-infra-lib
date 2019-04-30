ccola_component(event static_library)

    ccola_sources(
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

    ccola_dependencies(
        util
    )

ccola_end_component()

ccola_subdirectories(
    test_helper
    test
)
