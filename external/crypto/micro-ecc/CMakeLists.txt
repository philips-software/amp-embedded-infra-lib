add_library(crypto.micro_ecc ${EMIL_EXCLUDE_FROM_ALL} STATIC)
install(TARGETS crypto.micro_ecc EXPORT emilCryptoTargets)

target_include_directories(crypto.micro_ecc PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_options(crypto.micro_ecc PUBLIC
    $<$<BOOL:${TARGET_CORTEX}>:-fomit-frame-pointer>
    $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
)

target_sources(crypto.micro_ecc PRIVATE
    asm_arm_mult_square_umaal.inc
    asm_arm_mult_square.inc
    asm_arm.inc
    curve-specific.inc
    platform-specific.inc
    types.h
    uECC_vli.h
    uECC.c
    uECC.h
)

emil_exclude_from_clang_format(crypto.micro_ecc)
