add_library(services.synchronous_util ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(services.synchronous_util PUBLIC
    hal.interfaces
    hal.synchronous_interfaces
)

target_sources(services.synchronous_util PRIVATE
    SynchronousFlashAligner.cpp
    SynchronousFlashDelegate.cpp
    SynchronousFlashQuadSpi.cpp
    SynchronousFlashRegion.cpp
    SynchronousFlashSpi.cpp
    SynchronousSpiMasterWithChipSelect.cpp
    SynchronousFlashAligner.hpp
    SynchronousFlashDelegate.hpp
    SynchronousFlashQuadSpi.hpp
    SynchronousFlashRegion.hpp
    SynchronousFlashSpi.hpp
    SynchronousSpiMasterWithChipSelect.hpp
)

add_subdirectory(test)
