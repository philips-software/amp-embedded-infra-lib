add_executable(examples.serial_output)
target_sources(examples.serial_output PRIVATE Main.cpp)
target_link_libraries(examples.serial_output PRIVATE
    args
    hal.generic
    infra.stream
    services.tracer
)

if (EMIL_BUILD_WIN)
    target_link_libraries(examples.serial_output PRIVATE hal.windows)
endif()

if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
    target_link_libraries(examples.serial_output PRIVATE hal.unix)
endif()
