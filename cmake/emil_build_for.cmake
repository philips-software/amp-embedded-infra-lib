# 'emil_build_for' uses the EXCLUDE_FROM_ALL property on targets to specify whether the
# target should be built for a certain combination of properties.
#
# These properties are supported:
#
# HOST (values: All, Windows, Linux, Darwin): Build the target when building for any all of the host OSes
# TARGET_MCU_VENDOR: Build the target when building for this vendor
# TARGET_MCU_FAMILY: Build the target when building for this MCU family
# TARGET_MCU: Build the target when building for this MCU
# TARGET_MCU_VARIANT: Build the target when building for this MCU variant
# BOOL: Build the target when this boolean value evaluates to true
#
# PREREQUISITE_BOOL: Build the target only when this boolean value evaluates to true
# PREREQUISITE_CONFIG: Build the target only when building for this configuration
#
# The HOST, TARGET_*, and BOOL properties are ORed together. Its result is ANDed with the PREREQUISITE_* properties.
# Specifying PREREQUISITE_CONFIG results in a generator expression being used for EXCLUDE_FROM_ALL
#
# Examples:
# emil_buid_for(target HOST Linux)
# This will build target only for Linux
#
# emil_build_for(examples.clicking_scrolling HOST Windows TARGET_MCU stm32f746 PREREQUISITE_BOOL PREVIEW_BUILD_EXAMPLES)
# This will build examples.clicking_scrolling for Windows and for stm32f746, but only when building the preview examples

function(emil_build_for target)

    set(multiArgs HOST TARGET_MCU_VENDOR TARGET_MCU_FAMILY TARGET_MCU TARGET_MCU_VARIANT BOOL PREREQUISITE_BOOL PREREQUISITE_CONFIG)
    cmake_parse_arguments(PARSE_ARGV 1 BUILD_FOR "" "" "${multiArgs}")

    set(exclude TRUE)

    foreach(item ${BUILD_FOR_HOST})
        if (item STREQUAL All AND EMIL_HOST_BUILD)
            set(exclude FALSE)
        endif()
        if (item STREQUAL Windows AND EMIL_BUILD_WIN)
            set(exclude FALSE)
        endif()
        if (item STREQUAL Linux AND EMIL_BUILD_UNIX)
            set(exclude FALSE)
        endif()
        if (item STREQUAL Darwin AND EMIL_BUILD_DARWIN)
            set(exclude FALSE)
        endif()
    endforeach()

    foreach(attribute TARGET_MCU_VENDOR TARGET_MCU_FAMILY TARGET_MCU TARGET_MCU_VARIANT)
        foreach(item ${BUILD_FOR_${attribute}})
            if (item STREQUAL ${attribute})
                set(exclude FALSE)
            endif()
        endforeach()
    endforeach()

    foreach(item ${BUILD_FOR_BOOL})
        if (${${item}})
            set(exclude FALSE)
        endif()
    endforeach()

    foreach(item ${BUILD_FOR_PREREQUISITE_BOOL})
        if (NOT ${item})
            set(exclude TRUE)
        endif()
    endforeach()

    set(exclude_genex TRUE)
    set(uses_genex FALSE)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        set(exclude_${config} TRUE)
    endforeach()

    foreach(item ${BUILD_FOR_PREREQUISITE_CONFIG})
        set(exclude_genex "$<IF:$<CONFIG:${item}>,FALSE,${exclude_genex}>")
        set(uses_genex TRUE)
        set(exclude_${item} FALSE)
    endforeach()

    if (uses_genex AND NOT exclude)
        set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL ${exclude_genex})

        foreach(config ${CMAKE_CONFIGURATION_TYPES})
            set_target_properties(${target} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_${config} ${exclude_${config}})
        endforeach()
    else()
        set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL ${exclude})
    endif()

endfunction()

function(emil_install target)
    get_target_property(exclude ${target} EXCLUDE_FROM_ALL)
    if (NOT ${exclude})
        install(TARGETS ${target} ${ARGN})
    endif()
endfunction()
