ccola_component(synchronous_util static_library)

    ccola_sources(
        SynchronousFlashRegion.cpp
        SynchronousFlashQuadSpi.cpp
        SynchronousFlashSpi.cpp
        SynchronousSpiMasterWithChipSelect.cpp
        SynchronousFlashRegion.hpp
        SynchronousFlashQuadSpi.hpp
        SynchronousFlashSpi.hpp
        SynchronousSpiMasterWithChipSelect.hpp
    )

    ccola_dependencies(
        hal.synchronous_interfaces
    )

ccola_end_component()
