add_executable(infra.event_test)
add_test(NAME infra.event_test COMMAND infra.event_test)

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
