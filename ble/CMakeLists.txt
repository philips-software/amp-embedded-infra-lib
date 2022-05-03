ccola_package(ble)

    ccola_component(interfaces static_library)

        ccola_sources(
            dummy.cpp
            Gap.hpp
        )

        ccola_dependencies(
            infra.util
        )

    ccola_end_component()

ccola_end_package()
