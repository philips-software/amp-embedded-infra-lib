ccola_component(gmock x86: static_library other: source)

    ccola_sources(
        src/gmock.cc
        src/gmock-cardinalities.cc
        src/gmock-internal-utils.cc
        src/gmock-matchers.cc
        src/gmock-spec-builders.cc
        include/gmock/gmock.h
        include/gmock/gmock-actions.h
        include/gmock/gmock-cardinalities.h
        include/gmock/gmock-generated-actions.h
        include/gmock/gmock-generated-function-mockers.h
        include/gmock/gmock-generated-matchers.h
        include/gmock/gmock-generated-nice-strict.h
        include/gmock/gmock-matchers.h
        include/gmock/gmock-more-actions.h
        include/gmock/gmock-more-matchers.h
        include/gmock/gmock-spec-builders.h
        include/gmock/internal/gmock-generated-internal-utils.h
        include/gmock/internal/gmock-internal-utils.h
        include/gmock/internal/gmock-port.h
    )

    ccola_inheritable_include_directories(
        include
    )

    ccola_dependencies(
        gtest
    )

ccola_end_component()
