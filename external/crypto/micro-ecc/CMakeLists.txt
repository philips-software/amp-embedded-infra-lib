ccola_component(micro_ecc static_library)

    ccola_inheritable_include_directories(.)

    ccola_compile_options(win: "/wd4244" cortex: "-fomit-frame-pointer")

    ccola_sources(
        asm_arm.inc
        asm_arm_mult_square.inc
        curve-specific.inc
        platform-specific.inc
        types.h
        uECC.c
        uECC.h
        uECC_vli.h
    )

    ccola_subdirectories(
        #test
    )

ccola_end_component()
