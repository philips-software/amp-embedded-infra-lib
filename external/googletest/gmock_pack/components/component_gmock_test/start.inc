if(${CCOLA_ARCH} STREQUAL win)
	include("${CCOLA_PACK_DIR_cpp}/components/component_executable/start.inc")
	ccola_dependencies(
		gmock.gmock
		gmock.gtest_runner_with_leak_detection
	)
elseif(${CCOLA_ARCH} STREQUAL bsd)
	include("${CCOLA_PACK_DIR_cpp}/components/component_executable/start.inc")
	ccola_dependencies(
		gmock.gmock
		gmock.gtest_runner
	)
else()
	include("${CCOLA_PACK_DIR_cpp}/components/component_skip/start.inc")
endif()
