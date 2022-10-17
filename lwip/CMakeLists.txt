if (${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    target_compile_options(lwip.lwip PRIVATE -Wno-macro-redefined)
endif()

if (NOT LWIP_ALTERNATIVE_DEPEND)
    add_subdirectory(lwip_config)
    add_subdirectory(lwip-2.1.1)
endif()

add_subdirectory(lwip_cpp)
