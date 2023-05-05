add_library(osal.freertos_std_thread ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)

target_link_libraries(osal.freertos_std_thread INTERFACE
    freertos_kernel
    infra.util
)

target_include_directories(osal.freertos_std_thread INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_sources(osal.freertos_std_thread PRIVATE
    condition_variable
    mutex
    stdcondition_variable.hpp
    stdmutex.hpp
    stdonce.hpp
    stdthread.hpp
    thread
)

add_subdirectory(test)
