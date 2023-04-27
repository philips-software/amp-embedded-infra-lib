ccola_component(freertos_std_thread_test gmock_test)
    ccola_inheritable_include_directories(..)
	ccola_sources(
		FreeRTOS.h
		semphr.cpp
		semphr.h
		test_condition_variable.cpp
	)

	ccola_dependencies(
		freertos_std_thread
	)

ccola_end_component()
