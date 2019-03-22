if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "x86")
	set(CCOLA_COMPONENT_TEST 1)
	set(CCOLA_COMPONENT_TEST_PARAMS "--gtest_output=xml:${CCOLA_COMPONENT_NAME}.xml")

	ccola_translate_generator_expression(test_results_dir ${CCOLA_TEST_RESULTS_DIR})
	file(TO_NATIVE_PATH "${test_results_dir}" test_results_dir_n)
	set(CCOLA_COMPONENT_TEST_WORKING_DIR ${test_results_dir_n})

	set(save_exe_install_dir "${CCOLA_EXE_INSTALL_DIR}")
	set(CCOLA_EXE_INSTALL_DIR "${CCOLA_TEST_EXE_INSTALL_DIR}")

	ccola_translate_generator_expression(runtime_directories "${CCOLA_COMPONENT_RUNTIME_DIRECTORIES}")

	include("${CCOLA_PACK_DIR_cpp}/components/component_executable/end.inc")
	set(CCOLA_EXE_INSTALL_DIR "${save_exe_install_dir}")

	if (runtime_directories)
		file(TO_NATIVE_PATH ${runtime_directories} runtime_directories_n)
		set_property(TEST ${CCOLA_COMPONENT_NAME} PROPERTY ENVIRONMENT "PATH=${runtime_directories_n};$ENV{PATH}")
	endif()

	add_custom_command(
		TARGET ${CCOLA_COMPONENT_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "${test_results_dir}"
	)
else()
	include("${CCOLA_PACK_DIR_cpp}/components/component_skip/end.inc")
endif()
