macro(generate_xslt output)

	set(inputs ${ARGV})
	list(REMOVE_AT inputs 0)
	ccola_parse_selection_list(inputs)

	set(absolute_inputs)
	set(xslt_params)
	foreach(input ${inputs})
		string(SUBSTRING "${input}" 0 2 input_start)
		if(DEFINED xslt_params OR "${input_start}" STREQUAL "--")
			if(EXISTS ${input})
				if (CMAKE_HOST_WIN32)
					list(APPEND xslt_params "${input}")
				else()
					list(APPEND xslt_params "'${input}'")
				endif()
			else()
				list(APPEND xslt_params ${input})
			endif()
		else()
			ccola_make_absolute_from_source(absolute_input ${input})
			list(APPEND absolute_inputs "${absolute_input}")
			ccola_set_source_group("Source Files" "${absolute_input}")
		endif()
	endforeach()

	ccola_make_absolute_from_intermediate(absolute_output ${output})
	get_filename_component(absolute_directory "${absolute_output}" DIRECTORY)
	ccola_create_directory_on_demand(${absolute_directory})

	find_program(xsltproc_program
	    NAMES xsltproc
	          xsltproc.exe
	    HINTS ${CCOLA_PACK_DIR_xsltproc})

	add_custom_command(
		OUTPUT ${absolute_output}
		COMMAND ${xsltproc_program} ${xslt_params} --output "${absolute_output}" ${absolute_inputs}
		DEPENDS ${absolute_inputs}
	)

	ccola_sources(${absolute_inputs})

	unset(absolute_inputs)
	unset(xslt_params)

endmacro()
