cmake_minimum_required(VERSION 3.6.2)

project(emil VERSION 1.3.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if(NOT CCOLA_DIR)
    set(CCOLA_DIR "${CMAKE_SOURCE_DIR}/ccola")
endif()

if(NOT CCOLA_INSTALL_DIR)
	set(CCOLA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
endif()

include("${CCOLA_DIR}/ccola.inc")

ccola_project()

    if(CCOLA_ENABLE_COVERAGE)
        ccola_enable_coverage_flags()
    endif()

    ccola_add_version_header()

    ccola_subdirectories(
        external/googletest
        external/crypto
        infra
        hal
        services
        upgrade
        external/protobuf-3.3.0
        external/protoc-3.3.0
        protobuf
        lwip
        examples
    )

ccola_end_project()
