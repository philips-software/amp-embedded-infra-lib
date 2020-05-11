if(NOT EXISTS "${CCOLA_BUILD_DIR}/component_info/${item}.generic_cpp.inc")
	message(FATAL_ERROR "Dependency ${item} not found")
endif()
include("${CCOLA_BUILD_DIR}/component_info/${item}.generic_cpp.inc")

ccola_inheritable_dependencies(${CCOLA_DEPENDED_COMPONENT_DEPENDENCIES})
ccola_external_libraries(${CCOLA_DEPENDED_COMPONENT_EXTERNAL_LIBRARIES})
ccola_inheritable_include_directories(${CCOLA_DEPENDED_COMPONENT_INCLUDE_DIRECTORIES})

ccola_inheritable_link_libraries(${CCOLA_DEPENDED_COMPONENT_INHERITABLE_LINK_LIBRARIES})
