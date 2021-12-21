add_library(crypto.tiny_aes128 EXCLUDE_FROM_ALL STATIC)

target_include_directories(crypto.tiny_aes128 PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/../../"
)

target_sources(crypto.tiny_aes128 PRIVATE
    TinyAes.c
    TinyAes.h
)
