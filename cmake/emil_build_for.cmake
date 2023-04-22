function(emil_build_for)

    set(singleArgs TARGET)
    set(multiArgs HOST TARGET_MCU_VENDOR TARGET_MCU_FAMILY TARGET_MCU TARGET_MCU_VARIANT)
    cmake_parse_arguments(PARSE_ARGV 0 BUILD_FOR "" "${singleArgs}" "${multiArgs}")

    set(exclude TRUE)

    foreach(attribute TARGET_MCU_VENDOR TARGET_MCU_FAMILY TARGET_MCU TARGET_MCU_VARIANT)
        foreach(item ${BUILD_FOR_${attribute}})
            if (item STREQUAL ${attribute})
                set(exclude FALSE)
            endif()
        endforeach()
    endforeach()

    foreach(item ${BUILD_FOR_HOST})
        if (item STREQUALS All AND ${EMIL_HOST_BUILD})
            set(exclude FALSE)
        endif()
        if (item STREQUAL Windows AND ${EMIL_BUILD_WIN})
            set(exclude FALSE)
        endif()
        if (item STREQUAL Linux AND ${EMIL_BUILD_UNIX})
            set(exclude FALSE)
        endif()
        if (item STREQUAL Darwin AND ${EMIL_BUILD_DARWIN})
            set(exclude FALSE)
        endif()
    endforeach()

    set_property(TARGET ${BUILD_FOR_TARGET} PROPERTY EXCLUDE_FROM_ALL ${exclude})

endfunction()
