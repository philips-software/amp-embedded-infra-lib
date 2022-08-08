add_executable(examples.serial_output)
target_link_libraries(examples.serial_output PUBLIC
    args
    hal.generic
    infra.stream
    services.tracer
)

if (TARGET_BUILD_WIN)
endif()

if (TARGET_BUILD_UNIX OR TARGET_BUILD_DARWIN)
    target_sources(examples.serial_output PRIVATE MainUnix.cpp)
    target_link_libraries(examples.serial_output PUBLIC hal.unix)
endif()
