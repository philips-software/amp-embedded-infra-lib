include(${CMAKE_CURRENT_LIST_DIR}/toolchain-arm-gcc-m4.cmake)

add_link_options(-mfpu=fpv4-sp-d16 -mfloat-abi=hard)
add_compile_options(-mfpu=fpv4-sp-d16 -mfloat-abi=hard)
