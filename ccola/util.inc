macro(ccola_assert variable message_text)

	if(NOT ${variable})
		message(FATAL_ERROR ${message_text})
	endif()

endmacro()


macro(ccola_subdirectories)

	set(dirs "${ARGV}")
	ccola_parse_selection_list(dirs)

    foreach(item ${dirs})
		# Each subdirectory gets its own build dir at <intermediate build dir>/build/<number> by default,
		# so that we don't get a deep hierarchy of directories causing max path restrictions in Windows
		# to be violated when cmake uses paths relative to those deep paths.
		# In order to make certain make targets easily accessible, this behaviour can be overriden by
		# appending "=absolute_build_dir" to the directory name, which causes the path to become
		# <intermediate build dir>/<directory name>

		string(FIND ${item} "=absolute_build_dir" position REVERSE)
		if(NOT ${position} EQUAL -1)
			string(SUBSTRING ${item} 0 ${position} subdir)
			add_subdirectory(${subdir} "${CCOLA_BUILD_DIR}/${subdir}")
		else()
			if(DEFINED ${CCOLA_PACKAGE_NAME}_${item})
				math(EXPR ${CCOLA_PACKAGE_NAME}_${item} "${${CCOLA_PACKAGE_NAME}_${item}} + 1")
				add_subdirectory("${item}" "${CCOLA_BUILD_DIR}/build/${CCOLA_PACKAGE_NAME}.${item}.${${CCOLA_PACKAGE_NAME}_${item}}")
				ccola_promote_variable(${CCOLA_PACKAGE_NAME}_${item})
			else()
				add_subdirectory("${item}" "${CCOLA_BUILD_DIR}/build/${CCOLA_PACKAGE_NAME}.${item}")
				set(${CCOLA_PACKAGE_NAME}_${item} 0)
				ccola_promote_variable(${CCOLA_PACKAGE_NAME}_${item})
			endif()
		endif()
    endforeach()

	ccola_promote_variable(CCOLA_COMPONENT_TYPE_DIRS)
	ccola_promote_variable(CCOLA_PROGRAMMERS_DIRS)
	ccola_promote_variable(CCOLA_PACK_NAMES)
	foreach(pack_name ${CCOLA_PACK_NAMES})
		ccola_promote_variable(CCOLA_PACK_DIR_${pack_name})
	endforeach()
	ccola_promote_variable(CCOLA_GENERATED_EXECUTABLES)
	foreach(generated_executable ${CCOLA_GENERATED_EXECUTABLES})
		ccola_promote_variable(${generated_executable})
	endforeach()

endmacro()
set(CCOLA_UNIQUE_BUILD_DIR_NUMBER 0)


macro(ccola_register_parameter parameter)

    if(NOT DEFINED ${parameter})
        if(NOT "$ENV{${parameter}}" STREQUAL "")
            set(${${parameter}} "$ENV{${parameter}}" CACHE string "${parameter}")
        else()
            message(FATAL_ERROR "${parameter} is not defined in the environment")
        endif()
    else()
        set(${parameter} "${${parameter}}" CACHE string "${parameter}")
    endif()

endmacro()


macro(ccola_register_file_parameter parameter)

    if(NOT DEFINED ${parameter})
        message(FATAL_ERROR "${parameter} is not defined.")
    endif()

	get_filename_component(${parameter} ${${parameter}} ABSOLUTE)
    set(${parameter} "${${parameter}}" CACHE PATH "${parameter}" FORCE)

endmacro()


macro(ccola_validate_name name)

    if(NOT "${name}" MATCHES "^[a-zA-Z_][0-9a-zA-Z_]*$")
        message(FATAL_ERROR "${name} is not a valid name.")
    endif()

endmacro()


macro(ccola_verify_nested name var purpose nestpurpose)

    if(NOT DEFINED ${var})
        message(FATAL_ERROR "${purpose} ${name} is specified outside a ${nestpurpose}")
    endif()

endmacro()


macro(ccola_verify_not_nested name var purpose)

    if(DEFINED ${var})
        message(FATAL_ERROR "${purpose}s may not be nested: ${name} is nested in ${${var}}.")
    endif()

endmacro()


macro(ccola_verify_open name purpose)

    if(NOT DEFINED ${name})
        message(FATAL_ERROR "${purpose} has not been defined.")
    endif()
    
endmacro()


macro(ccola_verify_closed name purpose)

    if(DEFINED ${name})
        message(FATAL_ERROR "Cannot close ${purpose} when ${name} is still open.")
    endif()

endmacro()


macro(ccola_add_in_map map_name key value)

	# Append one key value pair to the map

    list(APPEND ${map_name} "${key}:${value}")

endmacro()


macro(ccola_replace_in_map map_name key value)

	unset(result_list)

	foreach(item ${${map_name}})
		if(NOT ${item} MATCHES "^${key}:")
			list(APPEND result_list ${item})
		endif()
	endforeach()

	list(APPEND result_list "${key}:${value}")
	set(${map_name} "${result_list}")

endmacro()


macro(ccola_append_map map_name key)

	# Append many values under the same key to the map

    set(values ${ARGV})
    list(REMOVE_AT values 0 1)
    
    foreach(item ${values})
        ccola_add_in_map(${map_name} ${key} ${item})
    endforeach()
    
endmacro()


macro(ccola_mapentry_get_key entry key_name)

	# Extract the key of a single mapentry (of the form "key:value")

    string(FIND ${entry} ":" position)
    string(SUBSTRING ${entry} 0 ${position} ${key_name})
    
endmacro()


macro(ccola_mapentry_get_value entry value_name)

	# Extract the value of a single mapentry (of the form "key:value")

    string(FIND ${entry} ":" position)
    string(LENGTH ${entry} size)
    math(EXPR startposition "${position} + 1")
    math(EXPR resultsize "${size} - ${position} - 1")
    string(SUBSTRING ${entry} ${startposition} ${resultsize} ${value_name})

endmacro()


macro(ccola_mapentry_split_key_value entry key_name value_name)

    ccola_mapentry_get_key(${item} ${key_name})
    ccola_mapentry_get_value(${item} ${value_name})

endmacro()


macro(ccola_parse output input)

	set(${output} "${input}")

endmacro()


macro(ccola_list_transform result list transform)

    set(${result})
    foreach(item ${${list}})
        list(APPEND ${result} "${transform}")
    endforeach()

endmacro()


macro(ccola_list_remove_duplicates list)

	# In contrast to the list(REMOVE_DUPLICATES ...) command, ccola_list_remove_duplicates also works on empty lists

	if(DEFINED ${list})
		list(REMOVE_DUPLICATES ${list})
	endif()

endmacro()


macro(ccola_list_get_back var args)

	list(LENGTH ${args} len)
	math(EXPR len "${len} - 1")
	list(GET ${args} ${len} ${var})

endmacro()


macro(ccola_item_matches_current_platform result item)
	
	if("${item}" STREQUAL "all:" OR "${item}" STREQUAL "${CCOLA_ARCH_FAMILY}:" OR "${item}" STREQUAL "${CCOLA_ARCH}:" OR "${item}" STREQUAL "${CCOLA_CORE}:" OR "${item}" STREQUAL "${CCOLA_PROCESSOR}:" OR "${item}" STREQUAL "${CCOLA_PROCESSOR_FAMILY}:")
		set(${result} TRUE)
	else()
		set(${result} FALSE)
	endif()

endmacro()


macro(ccola_parse_selection_list list)

	unset(selection_result)
	unset(selection_other)

	set(include_in_selection TRUE)

	foreach(item ${${list}})
		if(item MATCHES ":\$")
			ccola_item_matches_current_platform(match "${item}")
			if(${match})
				set(include_in_selection TRUE)
				set(include_in_other FALSE)
			elseif("${item}" STREQUAL "other:")
				set(include_in_selection FALSE)
				set(include_in_other TRUE)
			else()
				set(include_in_selection FALSE)
				set(include_in_other FALSE)
			endif()
		else()
			if(include_in_selection)
				list(APPEND selection_result "${item}")
			endif()
			if(include_in_other)
				list(APPEND selection_other "${item}")
			endif()
		endif()
	endforeach()

	if (selection_result)
		set(${list} "${selection_result}")
	else()
		set(${list} "${selection_other}")
	endif()

endmacro()


macro(ccola_parse_selection_list_with_params list_selected combinations list)

	unset(list_selected)
	unset(list_other)
	unset(${combinations})
	unset(combinations_other)

	set(include_in_selection TRUE)

	foreach(item ${${list}})
		if(item MATCHES ":\$")
			ccola_item_matches_current_platform(match ${item})
			if(${match})
				set(include_in_selection TRUE)
				set(include_in_other FALSE)
			elseif("${item}" STREQUAL "other:")
				set(include_in_selection FALSE)
				set(include_in_other TRUE)
			else()
				set(include_in_selection FALSE)
			endif()
		elseif(item MATCHES "\\?")
			string(REGEX REPLACE "\\?.*" "" condition ${item})
			string(REGEX REPLACE ".*\\?" "" property ${item})

			if("${condition}" STREQUAL "all" OR "${condition}" STREQUAL "${CCOLA_ARCH_FAMILY}" OR "${condition}" STREQUAL "${CCOLA_ARCH}" OR "${condition}" STREQUAL "${CCOLA_CORE}" OR "${condition}" STREQUAL "${CCOLA_PROCESSOR}"  OR "${condition}" STREQUAL "${CCOLA_PROCESSOR_FAMILY}")
				string(REGEX REPLACE "=.*" "" key ${property})
				string(REGEX REPLACE ".*=" "" value ${property})

				set(${key} ${value})
			endif()
		elseif(item MATCHES "=")
			string(REGEX REPLACE "=.*" "" key ${item})
			string(REGEX REPLACE ".*=" "" value ${item})

			set(${key} ${value})
		else()
			if(include_in_selection)
				list(APPEND ${list_selected} "${item}")
				list(APPEND ${combinations} ${list_selected})
			endif()
			if(include_in_other)
				list(APPEND ${list_other} "${item}")
				list(APPEND combinations_other ${list_other})
			endif()
		endif()
	endforeach()

	if(NOT ${list_selected})
		set(${list_selected} "${list_other}")
		set(${combinations} "${combinations_other}")
	endif()

	if(${combinations})
		list(REMOVE_DUPLICATES ${combinations})
	endif()

endmacro()


macro(ccola_promote_variable var)

	if(NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CCOLA_TOP_SOURCE_DIR}")
		set(${var} "${${var}}" PARENT_SCOPE)
	endif()

endmacro()
