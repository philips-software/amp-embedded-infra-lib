macro(ccola_csharp_project project)

	if(${CCOLA_ARCH} STREQUAL "win")
		set(sources ${ARGV})
		list(REMOVE_AT sources 0)

		include_external_msproject(${CCOLA_COMPONENT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${project})
		set_target_properties(${CCOLA_COMPONENT_NAME} PROPERTIES FOLDER ${CCOLA_PACKAGE_NAME})
		set(CCOLA_COMPONENT_PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
		set(CCOLA_COMPONENT_PROJECT_NAME ${project})

		ccola_get_filename_component_we(project_base ${project})
		set(source ${CMAKE_CURRENT_SOURCE_DIR}/bin/\$<configuration>/${project_base}.exe)
		ccola_translate_generator_expression(destination_dir "${CCOLA_COMPONENT_INSTALL_DIR}")
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${destination_dir}")

		unset(dependencies)
		foreach(sourcefile ${sources})
			ccola_make_absolute(sourcefile "${sourcefile}")
			list(APPEND dependencies ${sourcefile})
		endforeach()

		set(destination "${destination_dir}/${CCOLA_COMPONENT_NAME}.depend")
		configure_file(${CCOLA_PACK_DIR_csharp}/csharp_project.depend "${destination}")
	endif()

endmacro()
