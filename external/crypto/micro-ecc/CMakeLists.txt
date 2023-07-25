FetchContent_Declare(
    microecc
    GIT_REPOSITORY https://github.com/kmackay/micro-ecc
    GIT_TAG 24c60e243580c7868f4334a1ba3123481fe1aa48 # v1.1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

FetchContent_MakeAvailable(microecc)

add_library(crypto.micro_ecc ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_include_directories(crypto.micro_ecc PUBLIC
    "$<BUILD_INTERFACE:${microecc_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_options(crypto.micro_ecc PUBLIC
    $<$<BOOL:${TARGET_CORTEX}>:-fomit-frame-pointer>
    $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
)

target_sources(crypto.micro_ecc PRIVATE
    ${microecc_SOURCE_DIR}/asm_arm_mult_square_umaal.inc
    ${microecc_SOURCE_DIR}/asm_arm_mult_square.inc
    ${microecc_SOURCE_DIR}/asm_arm.inc
    ${microecc_SOURCE_DIR}/curve-specific.inc
    ${microecc_SOURCE_DIR}/platform-specific.inc
    ${microecc_SOURCE_DIR}/types.h
    ${microecc_SOURCE_DIR}/uECC_vli.h
    ${microecc_SOURCE_DIR}/uECC.c
    ${microecc_SOURCE_DIR}/uECC.h
)

emil_exclude_from_clang_format(crypto.micro_ecc)
