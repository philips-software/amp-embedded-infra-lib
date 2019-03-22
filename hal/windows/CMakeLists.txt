ccola_component(windows win: static_library other: noncompile)

    ccola_sources(
        FileSystemWin.cpp
        FileSystemWin.hpp
        SynchronousRandomDataGeneratorWin.hpp
        SynchronousRandomDataGeneratorWin.cpp
        TimerServiceWin.cpp
        TimerServiceWin.hpp
    )

    ccola_dependencies(
        interfaces
        synchronous_interfaces
    )

ccola_end_component()
