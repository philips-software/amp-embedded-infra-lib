add_subdirectory(echo)
add_subdirectory(echo_attributes)
add_subdirectory(echo_console)
add_subdirectory(protoc_echo)
add_subdirectory(protoc_echo_plugin)
add_subdirectory(protoc_echo_plugin_csharp)
add_subdirectory(protoc_echo_plugin_java)
add_subdirectory(services)

# Generate and install export file
install(EXPORT emilProtobufTargets
    FILE emilProtobufTargets.cmake
    NAMESPACE emil::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
)

# Generate the export targets for the build tree
export(EXPORT emilProtobufTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/emilProtobufTargets.cmake"
    NAMESPACE emil::
)

if (TARGET_ARCH_X86)
    # Generate and install export file
    install(EXPORT emilProtobufHostTargets
        FILE emilProtobufHostTargets.cmake
        NAMESPACE emil::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
    )

    # Generate the export targets for the build tree
    export(EXPORT emilProtobufHostTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/emilProtobufHostTargets.cmake"
        NAMESPACE emil::
    )
endif()
