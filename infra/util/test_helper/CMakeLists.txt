
add_library(infra.util_test_helper EXCLUDE_FROM_ALL STATIC)

target_link_libraries(infra.util_test_helper
    PUBLIC
    gmock
    infra.util
)

target_sources(infra.util_test_helper
    PRIVATE
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
