add_library(services.synchronous_util ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(services.synchronous_util PUBLIC
    hal.synchronous_interfaces
)

target_sources(services.synchronous_util PRIVATE
    SynchronousFlashRegion.cpp
    SynchronousFlashQuadSpi.cpp
    SynchronousFlashSpi.cpp
    SynchronousSpiMasterWithChipSelect.cpp
    SynchronousFlashRegion.hpp
    SynchronousFlashQuadSpi.hpp
    SynchronousFlashSpi.hpp
    SynchronousSpiMasterWithChipSelect.hpp
)

add_subdirectory(test)
