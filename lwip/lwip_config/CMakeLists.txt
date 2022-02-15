if (TARGET_BUILD_CORTEX)
    add_library(lwip.lwip_conf EXCLUDE_FROM_ALL STATIC)

    target_sources(lwip.lwip_conf PRIVATE
        sys_now.cpp
    )
else()
    add_library(lwip.lwip_conf EXCLUDE_FROM_ALL INTERFACE)
endif()

install(TARGETS lwip.lwip_conf EXPORT emilLwipTargets)

target_include_directories(lwip.lwip_conf INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
