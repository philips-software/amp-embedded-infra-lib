add_executable(examples.serial_net)
target_sources(examples.serial_net PRIVATE Main.cpp)
target_link_libraries(examples.serial_net PRIVATE
    hal.generic
    services.network_instantiations
    services.util
)

if (EMIL_BUILD_WIN)
    target_link_libraries(examples.serial_net PRIVATE hal.windows)
endif()

if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
    target_link_libraries(examples.serial_net PRIVATE hal.unix)
endif()
