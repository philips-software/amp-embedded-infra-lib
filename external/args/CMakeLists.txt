FetchContent_Declare(
    args
    GIT_REPOSITORY https://github.com/Taywee/args
    GIT_TAG e3e6e46699f1ce487a42fd64838f53daeb5aa89b # v6.4.6
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

set(ARGS_BUILD_EXAMPLE Off CACHE INTERNAL "")
set(ARGS_BUILD_UNITTESTS Off CACHE INTERNAL "")

FetchContent_MakeAvailable(args)

set_target_properties(args PROPERTIES FOLDER External/args)
install(TARGETS args EXPORT emilProtobufTargets)
