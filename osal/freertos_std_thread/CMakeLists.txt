add_library(osal.freertos_std_thread INTERFACE)
#install(TARGETS osal.freertos_std_thread EXPORT emilOsalTargets)

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

if (EMIL_BUILD_TESTS)
    #add_subdirectory(test)
endif()
