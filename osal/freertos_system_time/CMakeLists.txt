add_library(osal.freertos_system_time INTERFACE ${EMIL_EXCLUDE_FROM_ALL})
#install(TARGETS osal.freertos_system_time EXPORT emilOsalTargets)

target_link_libraries(osal.freertos_system_time INTERFACE
    freertos_kernel
    hal.interfaces
)

target_include_directories(osal.freertos_system_time INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../..>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_sources(osal.freertos_system_time PRIVATE
    TimerServiceFreeRtos.hpp
    TimerServiceFreeRtos.cpp
)
