add_library(osal.thread INTERFACE)

if (EMIL_INCLUDE_FREERTOS)
    add_subdirectory(freertos)
    add_subdirectory(freertos_std_thread)
    add_subdirectory(freertos_system_time)
    target_link_libraries(osal.thread INTERFACE osal.freertos_std_thread)
endif()

if (EMIL_INCLUDE_THREADX)
    add_subdirectory(threadx)
    add_subdirectory(threadx_std_thread)
    target_link_libraries(osal.thread INTERFACE osal.threadx_std_thread)
endif()

#install(TARGETS osal.thread EXPORT emilOsalTargets)
