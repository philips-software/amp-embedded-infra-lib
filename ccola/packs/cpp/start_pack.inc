macro(ccola_sources)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		unset(sources_selected_false_false_false)
		unset(sources_selected_false_true_false)
		unset(sources_selected_true_false_false)
		unset(sources_selected_true_true_false)
		unset(sources_selected_false_false_true)
		unset(sources_selected_false_true_true)
		unset(sources_selected_true_false_true)
		unset(sources_selected_true_true_true)

		set(sources ${ARGV})
		set(noncompile false)
		set(inheritable false)
		set(generated false)
		ccola_parse_selection_list_with_params(sources_selected_\${noncompile}_\${inheritable}_\${generated} sources_combinations sources)

		foreach(source_list ${sources_combinations})
			set(header_file_only FALSE)
			if(x${source_list} MATCHES "xsources_selected_true_.*")
				set(header_file_only TRUE)
			endif()

			set(generated FALSE)
			if(x${source_list} MATCHES "xsources_selected_.*_true")
				set(generated TRUE)
			endif()

			foreach(sourcefile ${${source_list}})
				list(APPEND CCOLA_COMPONENT_SOURCEFILES ${sourcefile})

				if(${generated})
					set_source_files_properties(${sourcefile} PROPERTIES GENERATED TRUE)
				endif()

				if(${header_file_only})
					set_source_files_properties(${sourcefile} PROPERTIES HEADER_FILE_ONLY TRUE)
				endif()

				get_filename_component(directory "${sourcefile}" DIRECTORY)
				
				# Strip off "public" and "private" from directory
				if(directory)
					get_filename_component(file "${directory}" NAME)
					if("${file}" STREQUAL "public" OR "${file}" STREQUAL "private")
						get_filename_component(directory "${directory}" DIRECTORY)
					endif()
				endif()

				if(${generated})
					ccola_set_source_group("Generated Source Files" ${sourcefile})
				elseif(directory)
					ccola_set_source_group("Source Files/${directory}" ${sourcefile})
				else()
					ccola_set_source_group("Source Files" ${sourcefile})
				endif()
			endforeach()

			if(x${source_list} MATCHES "xsources_selected_.*_true_.*")
				foreach(sourcefile ${${source_list}})
					ccola_make_absolute(absolute_source "${sourcefile}")
					list(APPEND CCOLA_COMPONENT_INHERITABLE_SOURCES "${absolute_source}")
				endforeach()
			endif()

		endforeach()

	endif()

endmacro()


macro(ccola_sources_for_executable)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		unset(sources_selected_false_false_false)
		unset(sources_selected_false_true_false)
		unset(sources_selected_true_false_false)
		unset(sources_selected_true_true_false)
		unset(sources_selected_false_false_true)
		unset(sources_selected_false_true_true)
		unset(sources_selected_true_false_true)
		unset(sources_selected_true_true_true)

		set(sources ${ARGV})
		set(noncompile false)
		set(generated false)
		ccola_parse_selection_list_with_params(sources_selected_\${noncompile}_\${generated} sources_combinations sources)

		foreach(source_list ${sources_combinations})
			if(x${source_list} MATCHES "xsources_selected_false_.*")
				foreach(sourcefile ${${source_list}})
					ccola_make_absolute(sourcefile "${sourcefile}")
					list(APPEND CCOLA_COMPONENT_SOURCEFILES_FOR_EXECUTABLE ${sourcefile})
				endforeach()
			elseif(x${source_list} MATCHES "xsources_selected_true_.*")
				foreach(sourcefile ${${source_list}})
					list(APPEND CCOLA_COMPONENT_SOURCEFILES_FOR_EXECUTABLE ${sourcefile})
					set_source_files_properties(${sourcefile} PROPERTIES HEADER_FILE_ONLY TRUE)
				endforeach()
			endif()

			if(x${source_list} MATCHES "xsources_selected_.*_true")
				foreach(sourcefile ${${source_list}})
					set_source_files_properties(${sourcefile} PROPERTIES GENERATED TRUE)
				endforeach()
			endif()
		endforeach()

		ccola_list_remove_duplicates(CCOLA_COMPONENT_SOURCEFILES_FOR_EXECUTABLE)

	endif()

endmacro()


macro(ccola_add_binary source symbol)

	# This macro adds a source file as binary data to an application.
	# Two symbols will be defined: ${symbol}_start and ${symbol}_end,
	# which point to the data.

	ccola_make_absolute_from_source(absolute_source "${source}")
	get_filename_component(path "${absolute_source}" DIRECTORY)
	get_filename_component(name "${absolute_source}" NAME)
	string(REPLACE "." "_" clean_name "${name}")

	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.o"
		COMMAND ${CCOLA_OBJ_COPY_TOOL} -I binary -O elf32-littlearm -B arm --redefine-sym _binary_${clean_name}_start=${symbol}_start --redefine-sym _binary_${clean_name}_end=${symbol}_end "${name}" "${CMAKE_CURRENT_BINARY_DIR}/${name}.o"
		DEPENDS "${absolute_source}"
		WORKING_DIRECTORY "${path}"
	)

	ccola_sources(
	cortex:
		"${CMAKE_CURRENT_BINARY_DIR}/${name}.o"
	)

endmacro()


macro(ccola_source_dependencies)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(dependencies "${ARGV}")
		ccola_parse_selection_list(dependencies)

		list(APPEND CCOLA_COMPONENT_SOURCE_DEPENDENCIES ${dependencies})
	endif()

endmacro()


macro(ccola_definitions)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(definitions "${ARGV}")
		ccola_parse_selection_list(definitions)

		list(APPEND CCOLA_COMPONENT_DEFINITIONS ${definitions})
		ccola_list_remove_duplicates(CCOLA_COMPONENT_DEFINITIONS)
	endif()

endmacro()


macro(ccola_inheritable_definitions)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(definitions "${ARGV}")
		ccola_parse_selection_list(definitions)

		ccola_definitions(${definitions})
		list(APPEND CCOLA_COMPONENT_INHERITABLE_DEFINITIONS ${definitions})
		ccola_list_remove_duplicates(CCOLA_COMPONENT_INHERITABLE_DEFINITIONS)
	endif()

endmacro()


macro(ccola_compile_options)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(options "${ARGV}")
		ccola_parse_selection_list(options)

		list(APPEND CCOLA_COMPONENT_COMPILE_OPTIONS ${options})
	endif()

endmacro()


macro(ccola_link_options)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(options "${ARGV}")
		ccola_parse_selection_list(options)

		list(APPEND CCOLA_COMPONENT_LINK_OPTIONS ${options})
	endif()

endmacro()


macro(ccola_cpp_cli)

	if(NOT CCOLA_COMPONENT_SKIP)
		set(CCOLA_COMPONENT_CPP_CLI 1)
	endif()

endmacro()


macro(ccola_exclude_from_all)

	if(NOT CCOLA_COMPONENT_SKIP)
		set(CCOLA_EXCLUDE_FROM_ALL EXCLUDE_FROM_ALL)
	endif()

endmacro()


macro(ccola_link_libraries)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(libraries "${ARGV}")
		ccola_parse_selection_list(libraries)

		foreach(item ${libraries})
			get_filename_component(full_path ${item} ABSOLUTE)
			list(APPEND CCOLA_COMPONENT_LINK_LIBRARIES ${full_path})
			ccola_list_remove_duplicates(CCOLA_COMPONENT_LINK_LIBRARIES)
		endforeach()
	endif()

endmacro()


macro(ccola_inheritable_link_libraries)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(libraries "${ARGV}")
		ccola_parse_selection_list(libraries)

		foreach(library ${libraries})
			ccola_translate_component_name(library)
			ccola_link_libraries(${library})
			get_filename_component(full_path ${library} ABSOLUTE)
			list(APPEND CCOLA_COMPONENT_INHERITABLE_LINK_LIBRARIES ${full_path})
		endforeach()
		ccola_list_remove_duplicates(CCOLA_COMPONENT_INHERITABLE_LINK_LIBRARIES)
	endif()

endmacro()


macro(ccola_library_directories)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(library_directories "${ARGV}")
		ccola_parse_selection_list(library_directories)

		foreach(item ${library_directories})
			ccola_make_absolute(path ${item})
			list(APPEND CCOLA_COMPONENT_LIBRARY_DIRECTORIES ${path})
		endforeach()

		ccola_list_remove_duplicates(CCOLA_COMPONENT_LIBRARY_DIRECTORIES)
	endif()

endmacro()


macro(ccola_external_libraries)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(libs ${ARGV})
		ccola_parse_selection_list(libs)

		foreach(item ${libs})
			list(APPEND CCOLA_COMPONENT_EXTERNAL_LIBRARIES ${item})
		endforeach()

		ccola_list_remove_duplicates(CCOLA_COMPONENT_EXTERNAL_LIBRARIES)
	endif()

endmacro()


macro(ccola_include_directories)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(directories "${ARGV}")
		ccola_parse_selection_list(directories)

		foreach(item ${directories})
			ccola_make_absolute(path ${item})
			list(APPEND paths ${path})
		endforeach()

		ccola_include_directories_absolute(${paths})
	endif()

endmacro()


macro(ccola_include_directories_absolute)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(directories "${ARGV}")
		ccola_parse_selection_list(directories)

		list(APPEND CCOLA_COMPONENT_INCLUDE_DIRECTORIES ${directories})
		ccola_list_remove_duplicates(CCOLA_COMPONENT_INCLUDE_DIRECTORIES)
	endif()

endmacro()


macro(ccola_inheritable_include_directories)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(dirs ${ARGV})
		ccola_parse_selection_list(dirs)

		unset(paths)
		foreach(item ${dirs})
			ccola_make_absolute(path "${item}")
			list(APPEND paths ${path})
		endforeach()

		ccola_inheritable_include_directories_absolute(${paths})
	endif()

endmacro()


macro(ccola_inheritable_include_directories_absolute)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(directories "${ARGV}")
		ccola_parse_selection_list(directories)

		ccola_include_directories_absolute(${directories})
		list(APPEND CCOLA_COMPONENT_INHERITABLE_INCLUDE_DIRECTORIES ${directories})
		ccola_list_remove_duplicates(CCOLA_COMPONENT_INHERITABLE_INCLUDE_DIRECTORIES)
	endif()

endmacro()


macro(ccola_runtime_directories)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(directories "${ARGV}")
		ccola_parse_selection_list(directories)

		foreach(item ${directories})
			ccola_make_absolute(path ${item})
			string(REPLACE "<CCOLA_COMPONENT_NAME>" ${CCOLA_COMPONENT_NAME} runtime_directory "${path}")

			list(APPEND CCOLA_COMPONENT_RUNTIME_DIRECTORIES "${runtime_directory}")
		endforeach()

		ccola_list_remove_duplicates(CCOLA_COMPONENT_RUNTIME_DIRECTORIES)
	endif()

endmacro()


macro(ccola_translate_component_name values)

		string(REPLACE "<CCOLA_COMPONENT_NAME>" "${CCOLA_COMPONENT_NAME}" ${values} "${${values}}")

endmacro()


macro(ccola_set_component_properties_for_all_configurations component property)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(values "${ARGV}")
		list(REMOVE_AT values 0 1)

		ccola_translate_component_name(values)

		foreach(CCOLA_CONFIGURATION ${CMAKE_CONFIGURATION_TYPES})
			string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}-${CCOLA_CONFIGURATION}" substituted_values "${values}")
			string(TOUPPER ${CCOLA_CONFIGURATION} configuration)
			set_target_properties(${component} PROPERTIES ${property}_${configuration} ${substituted_values})
		endforeach()

		string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" substituted_values "${values}")
		set_target_properties(${component} PROPERTIES ${property} ${substituted_values})
	endif()

endmacro()


macro(ccola_translate_generator_expression_with_component var component values)

	if(CMAKE_CONFIGURATION_TYPES)
		string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}-\$<CONFIGURATION>" ${var} "${values}")
	else()
		string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" ${var} "${values}")
	endif()
	string(REPLACE "<CCOLA_COMPONENT_NAME>" ${component} ${var} "${${var}}")

endmacro()


macro(ccola_translate_generator_expression var values)

	ccola_translate_generator_expression_with_component(${var} ${CCOLA_COMPONENT_NAME} "${values}")

endmacro()


macro(ccola_set_component_properties_with_generator_expression component property)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(values "${ARGV}")
		list(REMOVE_AT values 0 1)

		if(CMAKE_CONFIGURATION_TYPES)
			string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}-\$<CONFIGURATION>" values "${values}")
		else()
			string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" values "${values}")
		endif()
		set_target_properties(${component} PROPERTIES ${property} "${values}")
	endif()

endmacro()


macro(ccola_linker_scripts)

	if(NOT CCOLA_COMPONENT_SKIP)
		set(scripts "${ARGV}")
		ccola_parse_selection_list(scripts)
		foreach(script ${scripts})
			ccola_make_absolute(script_path "${script}")
			list(APPEND CCOLA_COMPONENT_LINKER_SCRIPTS "${script_path}")
			ccola_source_dependencies("${script_path}")
			ccola_set_source_group("Linker Scripts" "${script_path}")
		endforeach()
	endif()

endmacro()


macro(ccola_default_linker_scripts)

	if(NOT CCOLA_COMPONENT_SKIP)
		set(scripts "${ARGV}")
		ccola_parse_selection_list(scripts)
		foreach(script ${scripts})
			ccola_source_dependencies("${script_path}")
			ccola_set_source_group("Linker Scripts" "${script_path}")
		endforeach()

		ccola_inherited_default_linker_scripts(${scripts})
	endif()

endmacro()


macro(ccola_inherited_default_linker_scripts)

	if(NOT CCOLA_COMPONENT_SKIP)
		set(scripts "${ARGV}")
		ccola_parse_selection_list(scripts)
		foreach(script ${scripts})
			ccola_make_absolute(script_path "${script}")
			list(APPEND CCOLA_COMPONENT_DEFAULT_LINKER_SCRIPTS "${script_path}")
		endforeach()

		ccola_list_remove_duplicates(CCOLA_COMPONENT_DEFAULT_LINKER_SCRIPTS)
	endif()

endmacro()


macro(ccola_optimize optimize)

	if(${optimize} STREQUAL "size")
		set(CCOLA_OPTIMIZE size)
	elseif(${optimize} STREQUAL "debug")
		set(CCOLA_OPTIMIZE debug)
	elseif(${optimize} STREQUAL "none")
		set(CCOLA_OPTIMIZE none)
	else()
		message(FATAL_ERROR "Unrecognized optimization: ${optimize}")
	endif()

endmacro()


macro(ccola_programmer)

	set(programmer_name "${ARGV}")
	ccola_parse_selection_list(programmer_name)

	set(CCOLA_PROGRAMMER ${programmer_name})

endmacro()


macro(ccola_configure_eclipse_launch exe_path name)

	if(NOT CCOLA_COMPONENT_SKIP)
		set(friendly_name ${name})
		if(CCOLA_COMPONENT_NAME)
			set(friendly_name ${CCOLA_COMPONENT_NAME})
		endif()

		file(TO_NATIVE_PATH "${CCOLA_DEBUG_TOOL}" debug_tool_n)
		file(TO_NATIVE_PATH "${exe_path}" exe_path_n)
		get_filename_component(build_dir "${CCOLA_BUILD_DIR}" NAME)

		if(DEFINED CCOLA_EXTRA_CONFIGURATION)
			set(extra_configuration "-${CCOLA_EXTRA_CONFIGURATION}")
		else()
			set(extra_configuration "")
		endif()

		if(NOT CCOLA_PROGRAMMER)
			message(STATUS "No programmer specified, skipping generation of launch file for ${friendly_name}")
		else()
			ccola_find_last(programmer_file ${CCOLA_PROGRAMMER}_eclipse_${CCOLA_PROCESSOR}.launch.conf ${CCOLA_PROGRAMMERS_DIRS})
			if(NOT programmer_file)
				message(STATUS "Launch file for programmer ${CCOLA_PROGRAMMER} on ${CCOLA_PROCESSOR} not available, skipping generation of launch file for ${friendly_name}")
			else()
				file(RELATIVE_PATH friendly_programmer_file "${CCOLA_TOP_SOURCE_DIR}" "${programmer_file}")
				message(STATUS "Configuring launch file for ${friendly_name} from ${friendly_programmer_file}")
				configure_file("${programmer_file}" "${CCOLA_BUILD_DIR}/${name}${extra_configuration}-${CCOLA_PROCESSOR}.launch" @ONLY)

				file(APPEND "${CCOLA_BUILD_DIR}/eclipse_launch_files.txt" "${name}${extra_configuration}-${CCOLA_PROCESSOR}.launch\n")
			endif()
		endif()
	endif()

endmacro()


if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "lpc17xx")
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CCOLA_BUILD_DIR}/LaunchDummy")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CCOLA_PACK_DIR_cpp}/LaunchDummy" "${CCOLA_BUILD_DIR}/LaunchDummy")
endif()

if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "stm32f0xx")
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CCOLA_BUILD_DIR}/LaunchDummySTM32F0")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CCOLA_PACK_DIR_cpp}/LaunchDummySTM32F0" "${CCOLA_BUILD_DIR}/LaunchDummySTM32F0")
endif()

if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "stm32f4xx")
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CCOLA_BUILD_DIR}/LaunchDummySTM32F4")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CCOLA_PACK_DIR_cpp}/LaunchDummySTM32F4" "${CCOLA_BUILD_DIR}/LaunchDummySTM32F4")
endif()

if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "stm32f7xx")
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CCOLA_BUILD_DIR}/LaunchDummySTM32F7")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CCOLA_PACK_DIR_cpp}/LaunchDummySTM32F7" "${CCOLA_BUILD_DIR}/LaunchDummySTM32F7")
endif()

if(${CCOLA_PROCESSOR_FAMILY} STREQUAL "stm32wbxx")
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CCOLA_BUILD_DIR}/LaunchDummySTM32WB55")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CCOLA_PACK_DIR_cpp}/LaunchDummySTM32WB55" "${CCOLA_BUILD_DIR}/LaunchDummySTM32WB55")
endif()

if(DEFINED CCOLA_OPTIMIZE)
	ccola_optimize(${CCOLA_OPTIMIZE})
endif()

file(WRITE "${CCOLA_BUILD_DIR}/eclipse_launch_files.txt")
