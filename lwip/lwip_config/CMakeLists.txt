add_library(lwip.lwip_conf STATIC)
emil_build_for(lwip.lwip_conf PREREQUISITE_BOOL TARGET_CORTEX EMIL_STANDALONE)

target_include_directories(lwip.lwip_conf INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_sources(lwip.lwip_conf PRIVATE
    sys_now.cpp
)
