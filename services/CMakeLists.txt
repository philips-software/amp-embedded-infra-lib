add_subdirectory(cucumber)
add_subdirectory(network_bsd)
add_subdirectory(network_win)
add_subdirectory(network)
add_subdirectory(synchronous_util)
add_subdirectory(tracer)
add_subdirectory(util)

# Generate and install export file
install(EXPORT emilServicesTargets
    FILE emilServicesTargets.cmake
    NAMESPACE emil::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
)

# Generate the export targets for the build tree
export(EXPORT emilServicesTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/emilServicesTargets.cmake"
    NAMESPACE emil::
)
