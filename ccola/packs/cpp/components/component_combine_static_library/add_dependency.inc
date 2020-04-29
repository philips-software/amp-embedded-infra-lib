include("${CCOLA_BUILD_DIR}/component_info/${item}.generic_cpp.inc")

set(lib_name ${CMAKE_STATIC_LIBRARY_PREFIX}${CCOLA_COMPONENT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
string(REPLACE "<CCOLA_CONFIGURATION>" "${CCOLA_SYSTEM_CONFIGURATION}" lib_path "${CCOLA_STATIC_LIBRARY_INSTALL_DIR}")
string(REPLACE "<CCOLA_COMPONENT_NAME>" "${CCOLA_COMPONENT_NAME}" lib_path "${lib_path}")

ccola_inheritable_dependencies(${CCOLA_DEPENDED_COMPONENT_DEPENDENCIES})
ccola_external_libraries(${lib_path}/${lib_name})
ccola_inheritable_include_directories(${CCOLA_DEPENDED_COMPONENT_INCLUDE_DIRECTORIES})
ccola_inheritable_definitions(${CCOLA_DEPENDED_COMPONENT_INHERITABLE_DEFINITIONS})
ccola_inheritable_link_libraries(${CCOLA_DEPENDED_COMPONENT_INHERITABLE_LINK_LIBRARIES})