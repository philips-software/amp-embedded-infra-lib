include(${CMAKE_CURRENT_LIST_DIR}/toolchain-arm-gcc-m33.cmake)

add_link_options(-mfpu=fpv5-sp-d16 -mfloat-abi=hard)
add_compile_options(-mfpu=fpv5-sp-d16 -mfloat-abi=hard)
