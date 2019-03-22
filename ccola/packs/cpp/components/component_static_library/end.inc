include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_before.inc")

add_library(${CCOLA_COMPONENT_NAME} STATIC ${CCOLA_COMPONENT_SOURCEFILES} ${CCOLA_COMPONENT_SOURCE_DEPENDENCIES})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} ARCHIVE_OUTPUT_DIRECTORY ${CCOLA_STATIC_LIBRARY_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} LIBRARY_OUTPUT_DIRECTORY ${CCOLA_STATIC_LIBRARY_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} PDB_OUTPUT_DIRECTORY ${CCOLA_PDB_INSTALL_DIR})
set(lib_path ${CCOLA_STATIC_LIBRARY_INSTALL_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${CCOLA_COMPONENT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
ccola_inheritable_link_libraries(${lib_path})

if(${CCOLA_ARCH_FAMILY} STREQUAL cortex)
	string(REPLACE "<CCOLA_COMPONENT_NAME>" ${CCOLA_COMPONENT_NAME} lib_path "${lib_path}")
	string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" lib_path "${lib_path}")

	add_custom_command(
		TARGET ${CCOLA_COMPONENT_NAME}
		POST_BUILD
		COMMAND ${CCOLA_APP_SIZE_TOOL} ${lib_path}
	)
endif()

include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_after.inc")
