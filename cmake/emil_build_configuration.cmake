function(add_build_configuration_definition target)
    if (CMAKE_CONFIGURATION_TYPES)
        target_compile_definitions(${target} PRIVATE
            $<$<CONFIG:Debug>:BUILD_CONFIGURATION=\"Debug\"> 
            $<$<CONFIG:RelWithDebInfo>:BUILD_CONFIGURATION=\"RelWithDebInfo\"> 
            $<$<CONFIG:Release>:BUILD_CONFIGURATION=\"Release\"> 
            $<$<CONFIG:MinSizeRel>:BUILD_CONFIGURATION=\"MinSizeRel\"> 
        )
    else()
        if (NOT CMAKE_BUILD_TYPE)
            set(build_type "Unknown")
        else()
            set(build_type "${CMAKE_BUILD_TYPE}")
        endif()
        target_compile_definitions(${target} PRIVATE "BUILD_CONFIGURATION=\"${build_type}\"")
    endif()
endfunction()
