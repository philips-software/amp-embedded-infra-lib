if (EMIL_HOST_BUILD)
    add_library(hal.generic ${EMIL_EXCLUDE_FROM_ALL} STATIC)
    install(TARGETS hal.generic EXPORT emilHalTargets)

    target_link_libraries(hal.generic PUBLIC
        hal.interfaces
        hal.synchronous_interfaces
        $<$<OR:$<BOOL:${EMIL_BUILD_UNIX}>,$<BOOL:${EMIL_BUILD_DARWIN}>>:pthread>
    )
    
    target_sources(hal.generic PRIVATE
        FileSystemGeneric.cpp
        FileSystemGeneric.hpp
        SynchronousRandomDataGeneratorGeneric.cpp
        SynchronousRandomDataGeneratorGeneric.hpp
        TimeKeeperGeneric.cpp
        TimeKeeperGeneric.hpp
        TimerServiceGeneric.cpp
        TimerServiceGeneric.hpp
    )
endif()
