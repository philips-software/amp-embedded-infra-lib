# Three helper functions are introduced, the main helper function is folderize_all_targets()
# Using this function, all targets which have a name of the format package.target have their
# FOLDER property set to package, so that in supported IDEs those targets are combined into folders.

function(get_subdirectories foundDirectories topDir)
    set(found)
    get_property(subDirectories DIRECTORY ${topDir} PROPERTY SUBDIRECTORIES)
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

function(get_project_targets foundTargets)
    get_subdirectories(directories ${PROJECT_SOURCE_DIR})

    foreach(dir ${directories})
        get_property(subDirTargets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
        list(APPEND targets ${subDirTargets})
    endforeach()
    set(${foundTargets} ${targets} PARENT_SCOPE)
endfunction()

function(folderize_all_targets)
    get_project_targets(targets)

    foreach(target ${targets})
        string(REGEX MATCH [^\.]+ component ${target})
        if (NOT "${target}" STREQUAL "${component}")
            set_target_properties(${target} PROPERTIES FOLDER ${component})
        endif()
    endforeach()
endfunction()
