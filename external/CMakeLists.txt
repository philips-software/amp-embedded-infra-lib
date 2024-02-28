add_subdirectory(args)
add_subdirectory(crypto)
add_subdirectory(protobuf)
add_subdirectory(protoc)

if (EMIL_INCLUDE_SEGGER_RTT)
    add_subdirectory(segger_rtt)
endif()

emil_exclude_directory_from_clang_format(.)
