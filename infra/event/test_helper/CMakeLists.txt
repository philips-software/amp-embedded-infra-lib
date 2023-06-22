add_library(infra.event_test_helper INTERFACE)
emil_build_for(infra.event_test_helper BOOL BUILD_TESTING)

target_link_libraries(infra.event_test_helper INTERFACE
    gmock
    infra.event
    infra.util_test_helper
)

target_sources(infra.event_test_helper PRIVATE
    EventDispatcherFixture.hpp
    EventDispatcherWithWeakPtrFixture.hpp
)
