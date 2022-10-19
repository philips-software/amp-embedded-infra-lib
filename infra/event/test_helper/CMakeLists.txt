add_library(infra.event_test_helper ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)

target_link_libraries(infra.event_test_helper INTERFACE
    gmock
    infra.event
    infra.util_test_helper
)

target_sources(infra.event_test_helper PRIVATE
    EventDispatcherFixture.hpp
    EventDispatcherWithWeakPtrFixture.hpp
)

emil_clangformat_target(infra.event_test_helper)
