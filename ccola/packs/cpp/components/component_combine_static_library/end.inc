set(lib_name ${CMAKE_STATIC_LIBRARY_PREFIX}${CCOLA_COMPONENT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" lib_path "${CCOLA_STATIC_LIBRARY_INSTALL_DIR}")
string(REPLACE "<CCOLA_COMPONENT_NAME>" "${CCOLA_COMPONENT_NAME}" lib_path "${lib_path}")

foreach(item ${CCOLA_COMPONENT_EXCLUDE})
    if(CCOLA_COMPONENT_INHERITABLE_LINK_LIBRARIES)
        list(FILTER CCOLA_COMPONENT_INHERITABLE_LINK_LIBRARIES EXCLUDE REGEX ".*${item}.*")
    endif()
endforeach()

if(${CCOLA_ARCH} STREQUAL "win")
    get_filename_component(lib_tool_path ${CMAKE_LINKER} PATH)
    set(lib_tool lib.exe)

    foreach(item ${CCOLA_COMPONENT_INHERITABLE_LINK_LIBRARIES})
        string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" depend_lib "${item}")
        list(APPEND depend_list  "${depend_lib}")
        list(APPEND dependencies ${depend_lib})
    endforeach()

    add_custom_command(
        OUTPUT ${lib_path}/${lib_name}
        COMMAND ${CMAKE_COMMAND} -E remove ${lib_path}/${lib_name}
        COMMAND ${lib_tool_path}/${lib_tool} /OUT:${lib_path}/${lib_name} ${depend_list}
        DEPENDS ${depend_list}
    )
else()
    foreach(item ${CCOLA_COMPONENT_INHERITABLE_LINK_LIBRARIES})
        string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" depend_lib "${item}")
        list(APPEND extract_cmd COMMAND ${CMAKE_AR} -x ${depend_lib})
        list(APPEND dependencies ${depend_lib})
    endforeach()

    add_custom_command(
        OUTPUT ${lib_path}/${lib_name}
        COMMAND ${CMAKE_COMMAND} -E remove_directory obj
        COMMAND ${CMAKE_COMMAND} -E remove ${lib_path}/${lib_name}
        COMMAND ${CMAKE_COMMAND} -E make_directory obj
        COMMAND cd obj
        ${extract_cmd}
        COMMAND ${CMAKE_AR} -rc ${lib_path}/${lib_name} *
        DEPENDS ${dependencies}
    )
endif()

add_custom_target(${CCOLA_COMPONENT_NAME} ALL
    DEPENDS ${lib_path}/${lib_name}
)

ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} ARCHIVE_OUTPUT_DIRECTORY ${CCOLA_STATIC_LIBRARY_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} LIBRARY_OUTPUT_DIRECTORY ${CCOLA_STATIC_LIBRARY_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} PDB_OUTPUT_DIRECTORY ${CCOLA_PDB_INSTALL_DIR})
ccola_inheritable_link_libraries(${lib_path}/${lib_name})

configure_file(
    ${CCOLA_COMPONENT_GENERIC_CPP_INFO}
    ${CCOLA_COMPONENT_INFO_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}.generic_cpp.inc ESCAPE_QUOTES @ONLY)
configure_file(
    ${CCOLA_COMPONENT_GENERIC_CPP_PUBLIC_INFO}
    ${CCOLA_PUBLIC_COMPONENT_INFO_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}.generic_cpp.inc ESCAPE_QUOTES @ONLY)

unset(CCOLA_COMPONENT_INHERITABLE_INCLUDE_DIRECTORIES)
unset(CCOLA_COMPONENT_INCLUDE_DIRECTORIES)
unset(CCOLA_COMPONENT_LIBRARY_DIRECTORIES)
unset(CCOLA_COMPONENT_EXTERNAL_LIBRARIES)
unset(CCOLA_COMPONENT_LINK_LIBRARIES)
unset(CCOLA_COMPONENT_INHERITABLE_LINK_LIBRARIES)
unset(CCOLA_COMPONENT_DEFINITIONS)
unset(CCOLA_COMPONENT_INHERITABLE_DEFINITIONS)