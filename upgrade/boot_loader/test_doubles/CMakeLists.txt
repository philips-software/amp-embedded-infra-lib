ccola_component(boot_loader_test_doubles x86: static_library other: noncompile)

    ccola_sources(
        MockDecryptor.cpp
        MockDecryptor.hpp
    )

    ccola_dependencies(
        boot_loader
        gmock.gmock
    )

ccola_end_component()
