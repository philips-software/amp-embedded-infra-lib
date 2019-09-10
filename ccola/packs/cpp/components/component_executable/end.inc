include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_before.inc")

ccola_sources(${CCOLA_COMPONENT_SOURCEFILES_FOR_EXECUTABLE})

if(NOT DEFINED CCOLA_COMPONENT_LINKER_SCRIPTS)
	ccola_linker_scripts(${CCOLA_COMPONENT_DEFAULT_LINKER_SCRIPTS})
endif()

if(${CCOLA_ARCH_FAMILY} STREQUAL cortex OR ${CCOLA_ARCH_FAMILY} STREQUAL xtensa)
	set(linker_script_dummy "${CMAKE_CURRENT_BINARY_DIR}/linker_script_dummy.c")

	add_custom_command(
		OUTPUT "${linker_script_dummy}"
		COMMAND ${CMAKE_COMMAND} -E touch "${linker_script_dummy}"
		DEPENDS ${CCOLA_COMPONENT_LINKER_SCRIPTS}
	)

	ccola_source_dependencies("${linker_script_dummy}")

	if(NOT CCOLA_NO_NEWLIB_NANO)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")
	endif()
endif()

add_executable(${CCOLA_COMPONENT_NAME} ${CCOLA_COMPONENT_SUBSYSTEM} ${CCOLA_EXCLUDE_FROM_ALL} ${CCOLA_COMPONENT_SOURCEFILES} ${CCOLA_COMPONENT_SOURCE_DEPENDENCIES})

ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} RUNTIME_OUTPUT_DIRECTORY ${CCOLA_EXE_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} PDB_OUTPUT_DIRECTORY ${CCOLA_PDB_INSTALL_DIR})

if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "x86")
	if(${CCOLA_ARCH} STREQUAL "win")
    	set_target_properties(${CCOLA_COMPONENT_NAME} PROPERTIES ccola_location "${CCOLA_EXE_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}.exe")
    else()
    	set_target_properties(${CCOLA_COMPONENT_NAME} PROPERTIES ccola_location "${CCOLA_EXE_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}")
    endif()

    if(CMAKE_CONFIGURATION_TYPES)
		get_target_property(executable_path ${CCOLA_COMPONENT_NAME} RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO)
	else()
		get_target_property(executable_path ${CCOLA_COMPONENT_NAME} RUNTIME_OUTPUT_DIRECTORY)
	endif()
	set(executable_path ${executable_path}/${CCOLA_COMPONENT_NAME}${CMAKE_EXECUTABLE_SUFFIX})
	ccola_translate_generator_expression(destination_dir "${CCOLA_COMPONENT_INSTALL_DIR}")
	set(destination "${destination_dir}/${CCOLA_COMPONENT_NAME}.depend")
	set(dependencies)
	foreach(file ${CCOLA_COMPONENT_SOURCEFILES} ${CCOLA_COMPONENT_SOURCE_DEPENDENCIES})
		get_filename_component(path "${file}" ABSOLUTE)
		list(APPEND dependencies ${path})
	endforeach()

	configure_file(${CCOLA_PACK_DIR_cpp}/cpp_project.depend "${destination}")

	if(MSVC_IDE)
		string(REPLACE "<CCOLA_CONFIGURATION>" ${CCOLA_SYSTEM_CONFIGURATION}-Debug CCOLA_COMPONENT_RUNTIME_DIRECTORIES_DEBUG "${CCOLA_COMPONENT_RUNTIME_DIRECTORIES}")
		string(REPLACE "<CCOLA_CONFIGURATION>" ${CCOLA_SYSTEM_CONFIGURATION}-RelWithDebInfo CCOLA_COMPONENT_RUNTIME_DIRECTORIES_RELWITHDEBINFO "${CCOLA_COMPONENT_RUNTIME_DIRECTORIES}")
        file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" CCOLA_CURRENT_SOURCE_DIRECTORY)
		configure_file(${CCOLA_PACK_DIR_cpp}/components/component_executable/component.vcxproj.user.conf ${CMAKE_CURRENT_BINARY_DIR}/${CCOLA_COMPONENT_NAME}.vcxproj.user)
	endif()
endif()

if(${CCOLA_ARCH_FAMILY} STREQUAL cortex OR ${CCOLA_ARCH} STREQUAL linux)
	ccola_set_component_properties_with_generator_expression(${CCOLA_COMPONENT_NAME} LINK_LIBRARIES "-Wl,-(" ${CCOLA_COMPONENT_LINK_LIBRARIES} ${CCOLA_COMPONENT_EXTERNAL_LIBRARIES} -lgcc -lc -lg -lsupc++ "-Wl,-)" )
elseif(${CCOLA_ARCH_FAMILY} STREQUAL xtensa)
	ccola_set_component_properties_with_generator_expression(${CCOLA_COMPONENT_NAME} LINK_LIBRARIES "-Wl,-(" ${CCOLA_COMPONENT_LINK_LIBRARIES} ${CCOLA_COMPONENT_EXTERNAL_LIBRARIES} -lgcc -lc -lsupc++ "-Wl,-)" )
else()
	ccola_set_component_properties_with_generator_expression(${CCOLA_COMPONENT_NAME} LINK_LIBRARIES ${CCOLA_COMPONENT_LINK_LIBRARIES})
endif()

if(${CCOLA_ARCH_FAMILY} STREQUAL cortex OR ${CCOLA_ARCH_FAMILY} STREQUAL xtensa)
	set(mapfile ${CCOLA_EXE_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}.map)
	ccola_translate_generator_expression(mapfile "${mapfile}")
	set(linker_flags -Wl,-Map=\"${mapfile}\")

	if(CCOLA_COMPONENT_LINKER_SCRIPTS)
	    ccola_list_transform(script_params CCOLA_COMPONENT_LINKER_SCRIPTS "-Wl,-T\"\${item}\"")
	    string(REPLACE ";" " " script_params "${script_params}")
	    string(CONCAT linker_flags "${script_params}" " ${linker_flags}")
    endif()

	ccola_set_component_properties_with_generator_expression(${CCOLA_COMPONENT_NAME} LINK_FLAGS "${linker_flags}")

	set(exe_dir ${CCOLA_EXE_INSTALL_DIR})
	string(REPLACE "<CCOLA_COMPONENT_NAME>" ${CCOLA_COMPONENT_NAME} exe_dir "${exe_dir}")
	string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" exe_dir "${exe_dir}")
	set(exe_path "${exe_dir}/${CCOLA_COMPONENT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
	set(bin_path "${exe_dir}/${CCOLA_COMPONENT_NAME}.bin")
	set(hex_path "${exe_dir}/${CCOLA_COMPONENT_NAME}.hex")
	set(list_path "${exe_dir}/${CCOLA_COMPONENT_NAME}.lst")

	add_custom_command(
		TARGET ${CCOLA_COMPONENT_NAME}
		POST_BUILD
		COMMAND ${CCOLA_OBJ_DUMP_TOOL} --source --all-headers --demangle --line-numbers --wide "${exe_path}" > "${list_path}"
		COMMAND ${CCOLA_APP_SIZE_TOOL} "${exe_path}"
		COMMAND ${CCOLA_OBJ_COPY_TOOL} -O binary "${exe_path}" "${bin_path}"
		COMMAND ${CCOLA_OBJ_COPY_TOOL} -O ihex "${exe_path}" "${hex_path}"
		BYPRODUCTS ${bin_path} ${hex_path}
	)

	ccola_configure_eclipse_launch("${exe_path}" ${CCOLA_COMPONENT_NAME})
endif()

# Clear these flags so that they do not end up in the info.inc file
set(CCOLA_COMPONENT_INHERITABLE_INCLUDE_DIRECTORIES)
set(CCOLA_COMPONENT_LIBRARY_DIRECTORIES)
set(CCOLA_COMPONENT_LINK_LIBRARIES)

include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_after.inc")
