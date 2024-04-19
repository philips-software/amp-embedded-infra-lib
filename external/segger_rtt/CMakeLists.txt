FetchContent_Declare(
    segger_rtt
    GIT_REPOSITORY https://github.com/SEGGERMicro/RTT
    GIT_TAG ff57c3d516ea59b0a2d4d5cafb33a48572b09c0b # v7.54
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

FetchContent_MakeAvailable(segger_rtt)

add_library(segger.rtt ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_include_directories(segger.rtt PUBLIC
    "$<BUILD_INTERFACE:${segger_rtt_SOURCE_DIR}>"
)

target_sources(segger.rtt PRIVATE
    ${segger_rtt_SOURCE_DIR}/Config/SEGGER_RTT_Conf.h
    ${segger_rtt_SOURCE_DIR}/RTT/SEGGER_RTT.c
    ${segger_rtt_SOURCE_DIR}/RTT/SEGGER_RTT.h
)

emil_exclude_from_clang_format(segger.rtt)
