set(lib_name ${CMAKE_STATIC_LIBRARY_PREFIX}${CCOLA_COMPONENT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})

foreach(item ${CCOLA_COMPONENT_INHERITABLE_LINK_LIBRARIES})
    string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" depend_lib "${item}")
    list(APPEND extract_cmd COMMAND ${CMAKE_AR} -x ${depend_lib})
endforeach()

add_custom_command(
    OUTPUT combined_lib
    ${extract_cmd}
    COMMAND ${CMAKE_AR} -rc ${lib_name} *
    # COMMAND ${CMAKE_COMMAND} -E remove *.obj
)

add_custom_target(${CCOLA_COMPONENT_NAME} ALL
    DEPENDS combined_lib
)

#ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} ARCHIVE_OUTPUT_DIRECTORY ${CCOLA_STATIC_LIBRARY_INSTALL_DIR})
#ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} LIBRARY_OUTPUT_DIRECTORY ${CCOLA_STATIC_LIBRARY_INSTALL_DIR})
#ccola_set_component_properties_for_all_configurations(${CCOLA_COMPONENT_NAME} PDB_OUTPUT_DIRECTORY ${CCOLA_PDB_INSTALL_DIR})
#set(lib_path ${CCOLA_STATIC_LIBRARY_INSTALL_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${CCOLA_COMPONENT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
#ccola_inheritable_link_libraries(${lib_path})

include("${CCOLA_PACK_DIR_cpp}/components/generic_cpp/end_after.inc")