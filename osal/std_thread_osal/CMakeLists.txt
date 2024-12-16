add_library(osal.std_thread_osal ${EMIL_EXCLUDE_FROM_ALL} STATIC Osal.cpp)

target_include_directories(osal.std_thread_osal PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
    target_link_libraries(osal.std_thread_osal PUBLIC pthread)
endif()
