if(TARGET_ARCH_X86)
    add_library(hal.generic EXCLUDE_FROM_ALL STATIC)

    target_link_libraries(hal.generic
        PUBLIC

        hal.interfaces
        hal.synchronous_interfaces

        $<$<BOOL:${TARGET_BUILD_UNIX}>:pthread>
    )

    target_sources(hal.generic PRIVATE
        FileSystemGeneric.cpp
        FileSystemGeneric.hpp
        SynchronousRandomDataGeneratorGeneric.cpp
        SynchronousRandomDataGeneratorGeneric.hpp
        TimerServiceGeneric.cpp
        TimerServiceGeneric.hpp
    )
endif()
