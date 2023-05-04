add_executable(osal.freertos_std_thread_test)
emil_build_for(osal.freertos_std_thread_test PREREQUISITE_BOOL EMIL_BUILD_TESTS)
emil_add_test(osal.freertos_std_thread_test)

target_link_libraries(osal.freertos_std_thread_test PUBLIC
    osal.freertos_std_thread
)

target_sources(osal.freertos_std_thread_test PRIVATE
    FreeRTOS.h
    semphr.cpp
    semphr.h
    test_condition_variable.cpp
)
