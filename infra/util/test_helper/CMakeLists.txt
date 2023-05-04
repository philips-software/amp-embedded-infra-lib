add_library(infra.util_test_helper INTERFACE)
emil_build_for(infra.util_test_helper PREREQUISITE_BOOL BUILD_TESTING)

target_link_libraries(infra.util_test_helper INTERFACE
    gmock
    infra.util
)

target_sources(infra.util_test_helper PRIVATE
    BoundedStringMatcher.hpp
    OptionalMatcher.hpp
    LifetimeHelper.hpp
    MemoryRangeMatcher.hpp
    MockCallback.hpp
    MockHelpers.hpp
    MonitoredConstructionObject.hpp
    MoveConstructible.hpp
    ProxyCreatorMock.hpp
    VariantPrintTo.hpp
)
