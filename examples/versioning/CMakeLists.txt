add_executable(examples.versioning.c
    main.c
)

add_version_header_dependency(examples.versioning.c generated.version_header)


add_executable(examples.versioning.cpp
    main.cpp
)

target_link_libraries(examples.versioning.cpp PRIVATE
    infra.util
)

add_version_header_dependency(examples.versioning.cpp generated.version_header)
