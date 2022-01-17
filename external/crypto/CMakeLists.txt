add_subdirectory(mbedtls)
add_subdirectory(micro-ecc)
add_subdirectory(tiny-aes128)

# Generate and install export file
install(EXPORT emilCryptoTargets
    FILE emilCryptoTargets.cmake
    NAMESPACE emil::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
)

# Generate the export targets for the build tree
export(EXPORT emilCryptoTargets
    FILE "${CMAKE_BINARY_DIR}/emilCryptoTargets.cmake"
    NAMESPACE emil::
)
