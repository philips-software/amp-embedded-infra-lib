
add_library(infra.event_test_helper EXCLUDE_FROM_ALL STATIC)

target_link_libraries(infra.event_test_helper
    PUBLIC
    gmock
    infra.event
    infra.util_test_helper
)

target_sources(infra.event_test_helper
    PRIVATE
    EventDispatcherFixture.hpp
    EventDispatcherWithWeakPtrFixture.hpp
)
