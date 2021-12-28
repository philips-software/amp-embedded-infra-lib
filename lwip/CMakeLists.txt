if (NOT LWIP_ALTERNATIVE_DEPEND)
    add_subdirectory(lwip_config)
    add_subdirectory(lwip-2.1.1)
endif()

add_subdirectory(lwip_cpp)

# Generate and install export file
install(EXPORT emilLwipTargets
    FILE emilLwipTargets.cmake
    NAMESPACE emil::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
)

# Generate the export targets for the build tree
export(EXPORT emilLwipTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/emilLwipTargets.cmake"
    NAMESPACE emil::
)
