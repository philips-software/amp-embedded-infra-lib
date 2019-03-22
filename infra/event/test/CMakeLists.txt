ccola_component(event_test gmock_test)

    ccola_sources(
        TestClaimableResource.cpp
        TestEventHandler.cpp
        TestEventHandlerWithWeakPtr.cpp
        TestQueueForOneReaderOneIrqWriter.cpp
        TestSystemStateManager.cpp
    )

    ccola_dependencies(
        event
        event_test_helper
    )

ccola_end_component()
