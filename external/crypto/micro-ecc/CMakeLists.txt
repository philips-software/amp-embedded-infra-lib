add_library(crypto.micro_ecc EXCLUDE_FROM_ALL STATIC)
install(TARGETS crypto.micro_ecc EXPORT emilCryptoTargets)

target_include_directories(crypto.micro_ecc PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_options(crypto.micro_ecc PUBLIC
    $<$<BOOL:${TARGET_BUILD_CORTEX}>:-fomit-frame-pointer>
    $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
)

target_sources(crypto.micro_ecc PRIVATE
    asm_arm.inc
    asm_arm_mult_square.inc
    curve-specific.inc
    platform-specific.inc
    types.h
    uECC.c
    uECC.h
    uECC_vli.h
)
