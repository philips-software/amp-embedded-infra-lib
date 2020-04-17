macro(ccola_project)

	ccola_validate_name(${CMAKE_PROJECT_NAME})
	ccola_verify_not_nested(${CMAKE_PROJECT_NAME} CCOLA_PROJECT_NAME "Project")
	set(CCOLA_PROJECT_NAME ${CMAKE_PROJECT_NAME})

endmacro()


macro(ccola_end_project)

	ccola_verify_open(CCOLA_PROJECT_NAME "Project")
	ccola_verify_closed(CCOLA_PACKAGE_NAME "Package")

    get_property(pack_dirs GLOBAL PROPERTY CCOLA_PACK_DIRS)
	foreach(pack_dir ${pack_dirs})
		include(${pack_dir}/end_pack.inc)
	endforeach()

endmacro()


macro(ccola_package name)

	ccola_validate_name(${name})
	ccola_verify_not_nested(${name} CCOLA_PACKAGE_NAME "Package")
	set(CCOLA_PACKAGE_NAME ${name})

endmacro()


macro(ccola_end_package)

	ccola_verify_open(CCOLA_PACKAGE_NAME "Package")
	ccola_verify_closed(CCOLA_COMPONENT_NAME "Package")
	unset(CCOLA_PACKAGE_NAME)

endmacro()


macro(ccola_component name)

	ccola_validate_name(${name})
	ccola_verify_not_nested(${name} CCOLA_COMPONENT_NAME "Component")
	ccola_verify_nested(${name} CCOLA_PACKAGE_NAME "Component" "Package")

	set(componentlist "${ARGV}")
	list(REMOVE_AT componentlist 0)
	ccola_parse_selection_list(componentlist)
	set(componenttype ${componentlist})

	set(CCOLA_COMPONENT_NAME ${CCOLA_PACKAGE_NAME}.${name})
	set(CCOLA_COMPONENT_TYPE ${componenttype})
	message(STATUS "adding component ${CCOLA_COMPONENT_NAME} as ${CCOLA_COMPONENT_TYPE}")

	ccola_set_source_group(CMake "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")

	ccola_find_first(CCOLA_COMPONENT_TYPE_DIR component_${CCOLA_COMPONENT_TYPE} "${CCOLA_COMPONENT_TYPE_DIRS}")
	if(NOT CCOLA_COMPONENT_TYPE_DIR)
		message(FATAL_ERROR "Cannot find directory for component type ${CCOLA_COMPONENT_TYPE} in these directories: ${CCOLA_COMPONENT_TYPE_DIRS}")
	endif()

	ccola_require_file(${CCOLA_COMPONENT_TYPE_DIR}/start.inc "component type ${CCOLA_COMPONENT_TYPE}")
	ccola_require_file(${CCOLA_COMPONENT_TYPE_DIR}/end.inc "component type ${CCOLA_COMPONENT_TYPE}")
	include(${CCOLA_COMPONENT_TYPE_DIR}/start.inc)

	set_directory_properties(PROPERTIES
		INCLUDE_DIRECTORIES ""
		LINK_DIRECTORIES ""
		ADDITIONAL_MAKE_CLEAN_FILES ""
	)

endmacro()


macro(ccola_end_component)

	ccola_verify_open(CCOLA_COMPONENT_NAME "Component ${CCOLA_COMPONENT_NAME}")

	include(${CCOLA_COMPONENT_TYPE_DIR}/end.inc)

	unset(CCOLA_COMPONENT_NAME)
	unset(CCOLA_COMPONENT_TYPE)
	unset(CCOLA_COMPONENT_DEPENDENCIES)
	unset(CCOLA_COMPONENT_INHERITABLE_DEPENDENCIES)
	unset(CCOLA_COMPONENT_PUBLIC_PROPERTIES)

endmacro()


macro(ccola_inheritable_dependencies)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		list(APPEND CCOLA_COMPONENT_INHERITABLE_DEPENDENCIES ${ARGV})
		ccola_list_remove_duplicates(CCOLA_COMPONENT_INHERITABLE_DEPENDENCIES)
	endif()

endmacro()


macro(ccola_dependencies)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(dependencies "${ARGV}")
		ccola_parse_selection_list(dependencies)

		foreach(item ${dependencies})
			ccola_make_full_component_name(item)
			list(APPEND CCOLA_COMPONENT_DEPENDENCIES ${item})
			ccola_list_remove_duplicates(CCOLA_COMPONENT_DEPENDENCIES)
			include(${CCOLA_COMPONENT_TYPE_DIR}/add_dependency.inc)
		endforeach()
	endif()

endmacro()


macro(ccola_public_property property_name property_value)

	ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

	ccola_add_in_map(CCOLA_COMPONENT_PUBLIC_PROPERTIES ${property_name} ${property_value})

endmacro()


macro(ccola_read_public_property component_name property_name property_value_var)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(full_component_name ${component_name})
		ccola_make_full_component_name(full_component_name)

		ccola_require_file(${CCOLA_COMPONENT_INFO_INSTALL_DIR}/${full_component_name}.generic_cpp.inc "read public property of component ${full_component_name}")
		include(${CCOLA_COMPONENT_INFO_INSTALL_DIR}/${full_component_name}.generic_cpp.inc)

		foreach(item ${CCOLA_DEPENDED_COMPONENT_PUBLIC_PROPERTIES})
			ccola_mapentry_split_key_value(${item} key value)
			if(${key} STREQUAL ${property_name})
				set(${property_value_var} "${value}")
			endif()
		endforeach()
	endif()

endmacro()


macro(ccola_install_directory from to)

	install(DIRECTORY "${from}" DESTINATION "${to}")

endmacro()


macro(ccola_install_file from to)

	install(FILES "${from}" DESTINATION "${to}")

endmacro()


macro(ccola_install_file_rename from to rename)

	install(FILES "${from}" DESTINATION "${to}" RENAME "${rename}")

endmacro()


macro(ccola_pack_directory dir)

	get_filename_component(name "${dir}" NAME)
	message(STATUS "adding pack ${name}")

	if(NOT EXISTS ${dir})
		set(pack_dir "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	else()
		get_filename_component(pack_dir "${dir}" ABSOLUTE)
	endif()

	set(CCOLA_PACK_DIR_${name} "${pack_dir}")
	ccola_promote_variable(CCOLA_PACK_DIR_${name})
	ccola_require_file(${pack_dir}/start_pack.inc "pack ${name}")
	ccola_require_file(${pack_dir}/end_pack.inc "pack ${name}")
    get_property(pack_dirs GLOBAL PROPERTY CCOLA_PACK_DIRS)
	list(APPEND pack_dirs "${pack_dir}")
    set_property(GLOBAL PROPERTY CCOLA_PACK_DIRS ${pack_dirs})
	list(APPEND CCOLA_PACK_NAMES ${name})
	ccola_promote_variable(CCOLA_PACK_NAMES)
	include(${pack_dir}/start_pack.inc)

	ccola_make_absolute(components_dir "${pack_dir}/components")
	if(EXISTS "${components_dir}")
		list(APPEND CCOLA_COMPONENT_TYPE_DIRS "${components_dir}")
		ccola_promote_variable(CCOLA_COMPONENT_TYPE_DIRS)
	endif()

	ccola_make_absolute(programmers_dir "${pack_dir}/programmers")
	if(EXISTS "${programmers_dir}")
		list(APPEND CCOLA_PROGRAMMERS_DIRS "${programmers_dir}")
		ccola_promote_variable(CCOLA_PROGRAMMERS_DIRS)
	endif()

endmacro()


macro(ccola_make_full_component_name name_var)

	if(NOT ${${name_var}} MATCHES "^.+\\..+$")
		set(${name_var} "${CCOLA_PACKAGE_NAME}.${${name_var}}")
	endif()

endmacro()


macro(ccola_set_source_group group)

	if(NOT CCOLA_COMPONENT_SKIP)
		ccola_assert(CCOLA_COMPONENT_NAME "This command may only be used in a component")

		set(params ${ARGV})
		list(REMOVE_AT params 0)
		foreach(param ${params})
			ccola_add_in_map(CCOLA_COMPONENT_SOURCE_GROUP "${group}" "${param}")
		endforeach()
	endif()

endmacro()


macro(ccola_bin_dir result component_name)

	set(${result} ${CCOLA_EXE_INSTALL_DIR})
	string(REPLACE "<CCOLA_COMPONENT_NAME>" "${component_name}" ${result} "${${result}}")
	ccola_translate_generator_expression_with_component(${result} ${component_name} "${${result}}")

endmacro()


macro(ccola_get_executable_path path target)

	if(${CCOLA_ARCH} STREQUAL "win")
		get_target_property(${path} ${target} ccola_location)
        string(REPLACE "<CCOLA_COMPONENT_NAME>" "${target}" ${path} "${${path}}")
        ccola_translate_generator_expression(${path} "${${path}}")

		if(DEFINED CCOLA_COMPONENT_NAME)
			ccola_dependencies(
				${target}
			)
		endif()
	else()
		set(configuration RelWithDebInfo)
		set(depend_file "${CCOLA_COMPONENT_INSTALL_DIR}/${target}.depend")
		string(REPLACE "\$<configuration>" "${configuration}" depend_file "${depend_file}")
		string(REPLACE "<CCOLA_COMPONENT_NAME>" "${target}" depend_file "${depend_file}")

		include(${depend_file})
		set(${path} "${executable_path}")

		string(REPLACE "\$<configuration>" "${configuration}" ${path} "${${path}}")
		string(REPLACE "\$<configuration>" "${configuration}" generation_args "${generation_args}")

		#if (NOT generated_${path})
		#	set(generated_${path} 1)
        #
		#	add_custom_command(
		#		OUTPUT "${${path}}"
		#		COMMAND ${generation_command} ${generation_args}
		#		DEPENDS ${dependencies}
		#		VERBATIM
		#	)
        #
		#	ccola_promote_variable(generated_${path})
		#	list(APPEND CCOLA_GENERATED_EXECUTABLES generated_${path})
		#	ccola_promote_variable(CCOLA_GENERATED_EXECUTABLES)
		#endif()

	endif()

endmacro()


macro(ccola_enable_coverage_flags)

	if(${CCOLA_ARCH} STREQUAL "linux")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage -fno-inline")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage -fno-inline -fno-elide-constructors")

        if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
            link_libraries(gcov)
        else()
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        endif()

	endif()

endmacro()


define_property(TARGET PROPERTY ccola_location BRIEF_DOCS docs FULL_DOCS docs)
