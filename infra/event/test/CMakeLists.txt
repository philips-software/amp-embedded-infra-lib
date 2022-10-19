add_executable(infra.event_test)
emil_add_test(infra.event_test)

target_link_libraries(infra.event_test PUBLIC
    gmock_main
    infra.event
    infra.event_test_helper
)

target_sources(infra.event_test PRIVATE
    TestClaimableResource.cpp
    TestEventHandler.cpp
    TestEventHandlerWithWeakPtr.cpp
    TestQueueForOneReaderOneIrqWriter.cpp
    TestSystemStateManager.cpp
)

emil_clangformat_target(infra.event_test)
