add_library(crypto.tiny_aes128 ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_include_directories(crypto.tiny_aes128 PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_sources(crypto.tiny_aes128 PRIVATE
    TinyAes.c
    TinyAes.h
)

emil_exclude_from_clang_format(crypto.tiny_aes128)
