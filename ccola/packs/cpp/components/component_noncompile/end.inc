if(${CCOLA_ARCH} STREQUAL "win")
	include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_before.inc")

	ccola_sources(${CCOLA_COMPONENT_SOURCEFILES_FOR_EXECUTABLE})
	ccola_set_source_group("Source Files For Executable" "${CCOLA_COMPONENT_SOURCEFILES_FOR_EXECUTABLE}")

	foreach(sourcefile ${CCOLA_COMPONENT_SOURCEFILES} ${CCOLA_COMPONENT_SOURCE_DEPENDENCIES})
		set_source_files_properties(${sourcefile} PROPERTIES HEADER_FILE_ONLY TRUE)
	endforeach()

	ccola_create_dont_touch(${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
	ccola_set_source_group("CMake" "${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp")
	add_library(${CCOLA_COMPONENT_NAME} STATIC ${CCOLA_COMPONENT_SOURCEFILES} ${CCOLA_COMPONENT_SOURCE_DEPENDENCIES} ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)

	include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_after.inc")
else()
	ccola_make_absolute(CCOLA_COMPONENT_SKIP_CPP_INFO "${CCOLA_PACK_DIR_cpp}/components/component_noncompile/skip.inc.conf")

	configure_file(
		${CCOLA_COMPONENT_SKIP_CPP_INFO}
		${CCOLA_COMPONENT_INFO_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}.generic_cpp.inc ESCAPE_QUOTES @ONLY)
	configure_file(
		${CCOLA_COMPONENT_SKIP_CPP_INFO}
		${CCOLA_PUBLIC_COMPONENT_INFO_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}.generic_cpp.inc ESCAPE_QUOTES @ONLY)

	include("${CCOLA_PACK_DIR_cpp}/components/component_skip/end.inc")
endif()
