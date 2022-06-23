add_library(osal.thread INTERFACE)

if (EMIL_INCLUDE_FREERTOS)
    add_subdirectory(freertos)
    #target_link_libraries(osal.thread INTERFACE freertos_kernel)
endif()

install(TARGETS osal.thread EXPORT emilOsalTargets)
