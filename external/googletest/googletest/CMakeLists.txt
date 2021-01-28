ccola_component(gtest static_library)

    ccola_inheritable_definitions(_VARIADIC_MAX=10 GTEST_HAS_SEH=0)

    ccola_sources(
        src/gtest.cc
        src/gtest-death-test.cc
        src/gtest-filepath.cc
        src/gtest-internal-inl.h
        src/gtest-matchers.cc
        src/gtest-port.cc
        src/gtest-printers.cc
        src/gtest-test-part.cc
        src/gtest-typed-test.cc
        include/gtest/gtest.h
        include/gtest/gtest_pred_impl.h
        include/gtest/gtest_prod.h
        include/gtest/gtest-death-test.h
        include/gtest/gtest-matchers.h
        include/gtest/gtest-message.h
        include/gtest/gtest-param-test.h
        include/gtest/gtest-printers.h
        include/gtest/gtest-spi.h
        include/gtest/gtest-test-part.h
        include/gtest/gtest-typed-test.h
        include/gtest/internal/gtest-death-test-internal.h
        include/gtest/internal/gtest-filepath.h
        include/gtest/internal/gtest-internal.h
        include/gtest/internal/gtest-param-util.h
        include/gtest/internal/gtest-port.h
        include/gtest/internal/gtest-port-arch.h
        include/gtest/internal/gtest-string.h
        include/gtest/internal/gtest-type-util.h
        include/gtest/internal/custom/gtest.h
        include/gtest/internal/custom/gtest-port.h
        include/gtest/internal/custom/gtest-printers.h
    )

    ccola_inheritable_include_directories(
        include
    )

    ccola_include_directories(
        .
    )

ccola_end_component()


ccola_component(gtest_runner static_library)

    ccola_sources(
        src/gtest_main.cc
    )

    ccola_dependencies(
        gtest
    )

ccola_end_component()


ccola_component(gtest_runner_with_leak_detection win: static_library other: skip)

    ccola_sources(
        src/gtest_main_with_leak_detection.cc
    )

    ccola_dependencies(
        gtest
    )

ccola_end_component()
