add_subdirectory(boot_loader)
add_subdirectory(deploy_pack_to_external)
add_subdirectory(pack)
add_subdirectory(pack_builder)
add_subdirectory(security_key_generator)

if (EMIL_GENERATE_PACKAGE_CONFIG)
    # Generate and install export file
    install(EXPORT emilUpgradeTargets
        FILE emilUpgradeTargets.cmake
        NAMESPACE emil::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
    )

    # Generate the export targets for the build tree
    export(EXPORT emilUpgradeTargets
        FILE "${CMAKE_BINARY_DIR}/emilUpgradeTargets.cmake"
        NAMESPACE emil::
    )

    if (TARGET_ARCH_X86)
        # Generate and install export file
        install(EXPORT emilUpgradeHostTargets
            FILE emilUpgradeHostTargets.cmake
            NAMESPACE emil::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
        )

        # Generate the export targets for the build tree
        export(EXPORT emilUpgradeHostTargets
            FILE "${CMAKE_BINARY_DIR}/emilUpgradeHostTargets.cmake"
            NAMESPACE emil::
        )
    endif()
endif()
