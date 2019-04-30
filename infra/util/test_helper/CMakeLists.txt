ccola_component(util_test_helper x86: source other: skip)

    ccola_sources(
        BoundedStringMatcher.hpp
        OptionalMatcher.hpp
        MockCallback.hpp
        MockHelpers.hpp
        MonitoredConstructionObject.hpp
        MoveConstructible.hpp
        VariantPrintTo.hpp
    )

    ccola_dependencies(
        gmock.gmock
        util
    )

ccola_end_component()
