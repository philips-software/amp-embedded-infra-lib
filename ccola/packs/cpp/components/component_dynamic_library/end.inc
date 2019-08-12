include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_before.inc")

add_library(${CCOLA_COMPONENT_NAME} SHARED ${CCOLA_COMPONENT_SOURCEFILES} ${CCOLA_COMPONENT_SOURCE_DEPENDENCIES})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} RUNTIME_OUTPUT_DIRECTORY ${CCOLA_DLL_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} ARCHIVE_OUTPUT_DIRECTORY ${CCOLA_IMPORT_LIBRARY_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} LIBRARY_OUTPUT_DIRECTORY ${CCOLA_DLL_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} PDB_OUTPUT_DIRECTORY ${CCOLA_PDB_INSTALL_DIR})
ccola_set_component_properties_with_generator_expression(${CCOLA_COMPONENT_NAME} LINK_LIBRARIES ${CCOLA_COMPONENT_LINK_LIBRARIES})

if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "x86")
    set_target_properties(${CCOLA_COMPONENT_NAME} PROPERTIES ccola_location "${CCOLA_EXE_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")

    if(CMAKE_CONFIGURATION_TYPES)
		get_target_property(executable_path ${CCOLA_COMPONENT_NAME} RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO)
	else()
		get_target_property(executable_path ${CCOLA_COMPONENT_NAME} RUNTIME_OUTPUT_DIRECTORY)
	endif()

	set(executable_path ${executable_path}/${CCOLA_COMPONENT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

	ccola_translate_generator_expression(destination_dir "${CCOLA_COMPONENT_INSTALL_DIR}")
	set(destination "${destination_dir}/${CCOLA_COMPONENT_NAME}.depend")
	set(dependencies)
	foreach(file ${CCOLA_COMPONENT_SOURCEFILES} ${CCOLA_COMPONENT_SOURCE_DEPENDENCIES})
		get_filename_component(path "${file}" ABSOLUTE)
		list(APPEND dependencies ${path})
	endforeach()
	configure_file(${CCOLA_PACK_DIR_cpp}/cpp_project.depend "${destination}")
endif()

ccola_runtime_directories(${CCOLA_DLL_INSTALL_DIR})

if(${CCOLA_ARCH} STREQUAL "win")
    ccola_inheritable_link_libraries(${CCOLA_IMPORT_LIBRARY_INSTALL_DIR}/${CMAKE_IMPORT_LIBRARY_PREFIX}${CCOLA_COMPONENT_NAME}${CMAKE_IMPORT_LIBRARY_SUFFIX})
else()
    ccola_inheritable_link_libraries(${CCOLA_DLL_INSTALL_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${CCOLA_COMPONENT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_after.inc")
