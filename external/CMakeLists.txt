add_subdirectory(args SYSTEM)
add_subdirectory(crypto SYSTEM)
add_subdirectory(protobuf SYSTEM)
add_subdirectory(protoc SYSTEM)

if (EMIL_INCLUDE_SEGGER_RTT)
    add_subdirectory(segger_rtt SYSTEM)
endif()

emil_exclude_directory_from_clang_format(.)
emil_exclude_directory_from_coverage(.)
