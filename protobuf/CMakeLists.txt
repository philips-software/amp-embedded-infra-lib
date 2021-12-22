add_subdirectory(echo)
add_subdirectory(echo_attributes)
add_subdirectory(echo_console)
add_subdirectory(protoc_echo)
add_subdirectory(protoc_echo_plugin)
# TODO enable csharp and java plugin
# add_subdirectory(protoc_echo_plugin_csharp)
# add_subdirectory(protoc_echo_plugin_java)
add_subdirectory(services)

# Generate and install export file
install(EXPORT emilProtobufTargets
    FILE emilProtobuf.cmake
    NAMESPACE emil::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
)

# Generate the export targets for the build tree
export(EXPORT emilProtobufTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/emilProtobuf.cmake"
    NAMESPACE emil::
)
