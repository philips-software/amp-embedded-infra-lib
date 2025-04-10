if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        args
        GIT_REPOSITORY https://github.com/Taywee/args
        GIT_TAG 114200a9ad5fe06c8dea76e15d92325695cf3e34 # v6.4.7
    )

    set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

    set(ARGS_BUILD_EXAMPLE Off CACHE INTERNAL "")
    set(ARGS_BUILD_UNITTESTS Off CACHE INTERNAL "")

    FetchContent_MakeAvailable(args)

    set_target_properties(args PROPERTIES FOLDER External/args)
endif()
