add_executable(examples.versioning.c
    main.c
)

add_executable(examples.versioning.cpp
    main.cpp
)

add_version_header_dependency(examples.versioning.c generated.version_header)
add_version_header_dependency(examples.versioning.cpp generated.version_header)
