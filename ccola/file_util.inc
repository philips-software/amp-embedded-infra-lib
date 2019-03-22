macro(ccola_get_filename_component_we var path)

	string(FIND ${path} "." position REVERSE)
	string(SUBSTRING ${path} 0 ${position} ${var})
	get_filename_component(${var} ${${var}} NAME)

endmacro()


macro(ccola_get_filename_component_ext var path)

	string(FIND ${path} "." position REVERSE)
	string(SUBSTRING ${path} ${position} -1 ${var})

endmacro()


macro(ccola_make_absolute absolute_path_var path)

	get_filename_component(${absolute_path_var} ${path} ABSOLUTE)

endmacro()


macro(ccola_make_absolute_from_source absolute_path_var path)

	get_filename_component(absolute "${path}" ABSOLUTE)
	if("${absolute}" STREQUAL "${path}")
		set(${absolute_path_var} "${path}")
	else()
		get_filename_component(${absolute_path_var} "${CMAKE_CURRENT_SOURCE_DIR}/${path}" ABSOLUTE)
	endif()

endmacro()


macro(ccola_make_absolute_from_intermediate absolute_path_var path)

	get_filename_component(absolute "${path}" ABSOLUTE)
	if("${absolute}" STREQUAL "${path}")
		set(${absolute_path_var} "${path}")
	else()
		get_filename_component(${absolute_path_var} "${CMAKE_CURRENT_BINARY_DIR}/${path}" ABSOLUTE)
	endif()

endmacro()


macro(ccola_require_file file purpose)

    if(NOT EXISTS ${file})
        message(FATAL_ERROR "File ${file} is required for ${purpose}, but was not found.")
    endif()

endmacro()


macro(ccola_move_files files destination)

	file(GLOB files_to_move "${files}")
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${destination}")
	file(TO_NATIVE_PATH ${destination} destination_n)
	foreach(file ${files_to_move})
		file(TO_NATIVE_PATH ${file} file_n)
		get_filename_component(name ${file} NAME)
		execute_process(COMMAND ${CMAKE_COMMAND} -E remove "${destination}/${name}")
		execute_process(COMMAND ${CMAKE_COMMAND} -E rename "${file}" "${destination}")
	endforeach()

endmacro()


macro(ccola_remove_folder folder)

	file(REMOVE_RECURSE "${folder}")

endmacro()


macro(ccola_remove_files files)

	file(GLOB files_to_remove ${files})
	if(NOT ("${files_to_remove}" STREQUAL ""))
		file(REMOVE ${files_to_remove})
	endif()

endmacro()


macro(ccola_compare_restore files destination)

	file(GLOB files_to_restore ${files})
	file(TO_NATIVE_PATH ${destination} destination_n)
	foreach(file ${files_to_restore})
		get_filename_component(name ${file} NAME)
		if(EXISTS ${destination}/${name})
			file(READ ${file} contents1)
			file(READ ${destination}/${name} contents2)
			if("${contents1}" STREQUAL "${contents2}")
				file(TO_NATIVE_PATH ${file} file_n)
				execute_process(COMMAND ${CMAKE_COMMAND} -E remove "${destination}/${name}")
				execute_process(COMMAND ${CMAKE_COMMAND} -E rename "${file}" "${destination}")
			endif()
		endif()
	endforeach()

endmacro()


macro(ccola_find_first result file)

	set(paths ${ARGV})
	list(REMOVE_AT paths 0 1)
	unset(${result})
	foreach(path ${paths})
		if(EXISTS "${path}/${file}")
			set(${result} "${path}/${file}")
			break()
		endif()
	endforeach()

endmacro()


macro(ccola_find_last result file)

	set(paths ${ARGV})
	list(REMOVE_AT paths 0 1)
	unset(${result})
	foreach(path ${paths})
		if(EXISTS "${path}/${file}")
			set(${result} "${path}/${file}")
		endif()
	endforeach()

endmacro()


macro(ccola_copy source destination)

	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${source}" "${destination}")

endmacro()


macro(ccola_create_dont_touch file)

	if(NOT EXISTS "${file}")
		file(WRITE "${file}")
	endif()

endmacro()


macro(ccola_create_directory_on_demand directory)

	add_custom_command(
		OUTPUT "${directory}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${directory}"
	)

endmacro()
