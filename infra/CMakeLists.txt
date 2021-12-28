add_subdirectory(util)
add_subdirectory(stream)
add_subdirectory(syntax)
add_subdirectory(event)
add_subdirectory(timer)

# Generate and install export file
install(EXPORT emilInfraTargets
    FILE emilInfraTargets.cmake
    NAMESPACE emil::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
)

# Generate the export targets for the build tree
export(EXPORT emilInfraTargets
    FILE "${CMAKE_BINARY_DIR}/emilInfraTargets.cmake"
    NAMESPACE emil::
)
