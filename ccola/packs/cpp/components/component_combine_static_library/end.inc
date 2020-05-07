set(lib_name ${CMAKE_STATIC_LIBRARY_PREFIX}${CCOLA_COMPONENT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" lib_path "${CCOLA_STATIC_LIBRARY_INSTALL_DIR}")
string(REPLACE "<CCOLA_COMPONENT_NAME>" "${CCOLA_COMPONENT_NAME}" lib_path "${lib_path}")

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

add_custom_target(${CCOLA_COMPONENT_NAME} ALL
    DEPENDS ${lib_path}/${lib_name}
)

ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} ARCHIVE_OUTPUT_DIRECTORY ${CCOLA_STATIC_LIBRARY_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} LIBRARY_OUTPUT_DIRECTORY ${CCOLA_STATIC_LIBRARY_INSTALL_DIR})
ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} PDB_OUTPUT_DIRECTORY ${CCOLA_PDB_INSTALL_DIR})
set(lib_path ${CCOLA_STATIC_LIBRARY_INSTALL_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${CCOLA_COMPONENT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
ccola_inheritable_link_libraries(${lib_path})

configure_file(
    ${CCOLA_COMPONENT_GENERIC_CPP_INFO}
    ${CCOLA_COMPONENT_INFO_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}.generic_cpp.inc ESCAPE_QUOTES @ONLY)
configure_file(
    ${CCOLA_COMPONENT_GENERIC_CPP_PUBLIC_INFO}
    ${CCOLA_PUBLIC_COMPONENT_INFO_INSTALL_DIR}/${CCOLA_COMPONENT_NAME}.generic_cpp.inc ESCAPE_QUOTES @ONLY)