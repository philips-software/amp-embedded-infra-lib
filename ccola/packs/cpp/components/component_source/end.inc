include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_before.inc")

foreach(sourcefile ${CCOLA_COMPONENT_SOURCEFILES} ${CCOLA_COMPONENT_SOURCE_DEPENDENCIES})
	set_source_files_properties(${sourcefile} PROPERTIES HEADER_FILE_ONLY TRUE)
endforeach()

ccola_create_dont_touch(${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
ccola_set_source_group("CMake" "${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp")
add_library(${CCOLA_COMPONENT_NAME} STATIC ${CCOLA_COMPONENT_SOURCEFILES} ${CCOLA_COMPONENT_SOURCE_DEPENDENCIES} ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)

include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_after.inc")
