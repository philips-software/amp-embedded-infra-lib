include(${CMAKE_CURRENT_LIST_DIR}/toolchain-arm-gcc.cmake)

add_link_options(-mcpu=cortex-m0plus)
add_compile_options(-mcpu=cortex-m0plus)
