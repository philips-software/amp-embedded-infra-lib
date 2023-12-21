# Three helper functions are introduced, the main helper function is emil_folderize_all_targets()
# Using this function, all targets which have a name of the format package.target have their
# FOLDER property set to package, so that in supported IDEs those targets are combined into folders.

function(emil_get_subdirectories foundDirectories)
    set(multiValueArgs DIRECTORIES)
    cmake_parse_arguments(PARSE_ARGV 1 MY "" "" "${multiValueArgs}")

    set(found)
    set(subDirectories)
    foreach(dir ${MY_DIRECTORIES})
        get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
        list(APPEND subDirectories ${subdirs})
    endforeach()
    list(REMOVE_DUPLICATES subDirectories)

    while(subDirectories)
        list(POP_BACK subDirectories directory)
        list(APPEND found ${directory})
        get_property(directSubDirectories DIRECTORY ${directory} PROPERTY SUBDIRECTORIES)
        if(directSubDirectories)
            list(APPEND subDirectories ${directSubDirectories})
        endif()
    endwhile()
    set(${foundDirectories} ${found} PARENT_SCOPE)
endfunction()

function(emil_get_targets_from_directories foundTargets)
    set(multiValueArgs DIRECTORIES)
    cmake_parse_arguments(PARSE_ARGV 1 TARGETS_FROM "" "" "${multiValueArgs}")

    foreach(dir ${TARGETS_FROM_DIRECTORIES})
        get_property(subDirTargets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
        list(APPEND targets ${subDirTargets})
    endforeach()
    set(${foundTargets} ${targets} PARENT_SCOPE)
endfunction()

function(emil_get_project_targets foundTargets)
    emil_get_subdirectories(directories DIRECTORIES ${PROJECT_SOURCE_DIR})
    emil_get_targets_from_directories(targets DIRECTORIES ${directories})
    set(${foundTargets} "${targets}" PARENT_SCOPE)
endfunction()

function(emil_folderize_all_targets)
    emil_get_project_targets(targets)

    foreach(target ${targets})
        string(REGEX MATCH [^\.]+ component ${target})
        if (NOT "${target}" STREQUAL "${component}")
            set_target_properties(${target} PROPERTIES FOLDER ${component})
        endif()
    endforeach()
endfunction()
