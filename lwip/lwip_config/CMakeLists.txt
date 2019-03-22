ccola_component(lwip_conf static_library)

    ccola_inheritable_include_directories(
        .
    )

    ccola_sources(
        arch/cc.h
        arch/perf.h
        lwipopts.h
    win?noncompile=true
        sys_now.cpp
    )

ccola_end_component()
