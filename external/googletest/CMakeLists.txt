if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "x86")
    ccola_package(gmock)

        ccola_subdirectories(
            googletest
            googlemock
        )

    ccola_end_package()

endif()

ccola_pack_directory(gmock_pack)
