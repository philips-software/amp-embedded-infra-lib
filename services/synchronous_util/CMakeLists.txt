add_library(services.synchronous_util ${EMIL_EXCLUDE_FROM_ALL} STATIC)
install(TARGETS services.synchronous_util EXPORT emilServicesTargets)

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

emil_clangformat_target(services.synchronous_util)

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
