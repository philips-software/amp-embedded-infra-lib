add_subdirectory(args SYSTEM)
add_subdirectory(crypto SYSTEM)
add_subdirectory(protobuf SYSTEM)
add_subdirectory(protoc SYSTEM)

if (EMIL_INCLUDE_SEGGER_RTT)
    add_subdirectory(segger_rtt SYSTEM)
endif()

emil_exclude_directory_from_clang_format(.)
emil_exclude_directory_from_coverage(.)

function(remove_compile_options target options)
    get_target_property(all_options ${target} COMPILE_OPTIONS)
    if(all_options)
        list(REMOVE_ITEM all_options ${options})
        set_target_properties(libprotobuf PROPERTIES COMPILE_OPTIONS "${all_options}")
    endif()
endfunction()

# /MP is added by protobuf by checking for MSVC, but that check is a (partial) false positive since we use the MSVC _frontend_ for clang
# which does not support this flag
remove_compile_options(libprotobuf "/MP")
remove_compile_options(libprotobuf-lite "/MP")
remove_compile_options(libprotoc "/MP")
remove_compile_options(libupb "/MP")
