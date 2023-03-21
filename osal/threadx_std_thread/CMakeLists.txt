add_library(osal.threadx_std_thread ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)
#install(TARGETS osal.threadx_std_thread EXPORT emilOsalTargets)

target_link_libraries(osal.threadx_std_thread INTERFACE
    threadx
    infra.util
)

target_include_directories(osal.threadx_std_thread INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_sources(osal.threadx_std_thread PRIVATE
    condition_variable
    mutex
    stdcondition_variable.hpp
    stdmutex.hpp
    stdonce.hpp
    stdthread.hpp
    thread
)
