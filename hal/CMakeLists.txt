add_subdirectory(interfaces)
add_subdirectory(synchronous_interfaces)
add_subdirectory(generic)
add_subdirectory(windows)

# Generate and install export file
install(EXPORT emilHalTargets
    FILE emilHalTargets.cmake
    NAMESPACE emil::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
)

# Generate the export targets for the build tree
export(EXPORT emilHalTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/emilHalTargets.cmake"
    NAMESPACE emil::
)
