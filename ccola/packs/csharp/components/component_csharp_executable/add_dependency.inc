if(NOT EXISTS "${CCOLA_BUILD_DIR}/component_info/${item}.csharp_executable.inc")
	message(FATAL_ERROR "Dependency ${item} not found")
endif()
include("${CCOLA_BUILD_DIR}/component_info/${item}.csharp_executable.inc")

ccola_inheritable_dependencies(${CCOLA_DEPENDED_COMPONENT_DEPENDENCIES})
ccola_inheritable_include_directories(${CCOLA_DEPENDED_COMPONENT_INCLUDE_DIRECTORIES})
ccola_inheritable_definitions(${CCOLA_DEPENDED_COMPONENT_INHERITABLE_DEFINITIONS})
