add_executable(infra.event_test)
gtest_discover_tests(infra.event_test AUTO)

target_link_libraries(infra.event_test
    PUBLIC
    infra.event
    infra.event_test_helper

    gtest
    gmock
    gmock_main
)

target_sources(infra.event_test
    PRIVATE
    TestClaimableResource.cpp
    TestEventHandler.cpp
    TestEventHandlerWithWeakPtr.cpp
    TestQueueForOneReaderOneIrqWriter.cpp
    TestSystemStateManager.cpp
)
