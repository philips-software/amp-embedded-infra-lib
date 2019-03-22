if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "x86")
	include("${CCOLA_PACK_DIR_cpp}/components/component_executable/start.inc")
	ccola_dependencies(
		gmock.gmock
		gmock.gtest_runner
	)
else()
	include("${CCOLA_PACK_DIR_cpp}/components/component_skip/start.inc")
endif()
