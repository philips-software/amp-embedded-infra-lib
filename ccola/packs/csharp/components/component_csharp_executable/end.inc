if(${CCOLA_ARCH} STREQUAL "win")
	ccola_get_filename_component_we(name ${CCOLA_COMPONENT_PROJECT_NAME})

	foreach(configuration ${CMAKE_CONFIGURATION_TYPES})
		if(DEFINED ccola_location)
			set_target_properties(${CCOLA_COMPONENT_NAME} PROPERTIES ccola_location "${ccola_location}")
		else()
			set_target_properties(${CCOLA_COMPONENT_NAME} PROPERTIES ccola_location "${CCOLA_COMPONENT_PROJECT_PATH}/bin/$<CONFIGURATION>/${name}.exe")
		endif()
	endforeach()

	configure_file(
		"${CCOLA_COMPONENT_CSHARP_EXECUTABLE_INFO}"
		"${CCOLA_COMPONENT_INFO_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}.generic_cpp.inc" ESCAPE_QUOTES @ONLY)

	unset(CCOLA_COMPONENT_PROJECT_PATH)
	unset(CCOLA_COMPONENT_PROJECT_NAME)
endif()
