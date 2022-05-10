FetchContent_Declare(
    args
    GIT_REPOSITORY https://github.com/Taywee/args.git
    GIT_TAG 730dfbc4bc2e4149c22e0f606bf00420b65aeaeb
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

set(ARGS_BUILD_EXAMPLE Off CACHE INTERNAL "")
set(ARGS_BUILD_UNITTESTS Off CACHE INTERNAL "")

FetchContent_MakeAvailable(args)

set_target_properties(args PROPERTIES FOLDER External/args)
