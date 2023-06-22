add_library(hal.generic STATIC)
emil_build_for(hal.generic HOST Windows PREREQUISITE_BOOL EMIL_STANDALONE)

target_link_libraries(hal.generic PUBLIC
    hal.interfaces
    hal.synchronous_interfaces
    $<$<OR:$<BOOL:${EMIL_BUILD_UNIX}>,$<BOOL:${EMIL_BUILD_DARWIN}>>:pthread>
)

target_sources(hal.generic PRIVATE
    FileSystemGeneric.cpp
    FileSystemGeneric.hpp
    SerialCommunicationConsole.cpp
    SerialCommunicationConsole.hpp
    SynchronousRandomDataGeneratorGeneric.cpp
    SynchronousRandomDataGeneratorGeneric.hpp
    TimeKeeperGeneric.cpp
    TimeKeeperGeneric.hpp
    TimerServiceGeneric.cpp
    TimerServiceGeneric.hpp
)
