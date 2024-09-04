define_property(TARGET PROPERTY EXCLUDE_FROM_CLANG_FORMAT)

function(emil_exclude_from_clang_format target)
    set_target_properties(${target} PROPERTIES EXCLUDE_FROM_CLANG_FORMAT TRUE)
endfunction()

function(emil_exclude_directory_from_clang_format directory)
    emil_get_subdirectories(directories DIRECTORIES ${directory})
    list(APPEND directories ${directory})
    emil_get_targets_from_directories(targets DIRECTORIES ${directories})

    foreach(target ${targets})
        emil_exclude_from_clang_format(${target})
    endforeach()
endfunction()

function(emil_clangformat_directories postfix directories)
    set(singleArgs CLANG_FORMAT_FILE)
    set(multiValueArgs DIRECTORIES)
    cmake_parse_arguments(PARSE_ARGV 1 MY "" "${singleArgs}" "${multiValueArgs}")

    emil_get_subdirectories(subdirectories DIRECTORIES ${MY_DIRECTORIES})
    emil_get_targets_from_directories(targets DIRECTORIES ${subdirectories})

    set(non_generated_target_sources)
    foreach(target ${targets})
        get_target_property(exclude ${target} EXCLUDE_FROM_CLANG_FORMAT)
        if (NOT exclude)
            get_target_property(target_source_dir ${target} SOURCE_DIR)
            get_target_property(target_sources ${target} SOURCES)
            if (target_sources)
                foreach(source ${target_sources})
                    cmake_path(IS_RELATIVE source is_relative)
                    if (is_relative)
                        string(FIND "${source}" "$<" has_genex)
                        if (NOT has_genex EQUAL -1)
                            set(source_path $<$<BOOL:${source}>:${target_source_dir}/${source}>)
                        else()
                            set(source_path ${target_source_dir}/${source})
                        endif()
                    else()
                        set(source_path ${source})
                    endif()
                    get_source_file_property(generated ${source} TARGET_DIRECTORY ${target} GENERATED)
                    if (NOT generated)
                        list(APPEND non_generated_target_sources ${source_path})
                    endif()
                endforeach()
            endif()
        endif()
    endforeach()

    find_program(CLANGFORMAT clang-format)
    if (CLANGFORMAT)
        add_custom_target(clangformat_${postfix})

        if (NOT DEFINED MY_CLANG_FORMAT_FILE)
            set(MY_CLANG_FORMAT_FILE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../.clang-format)
        endif()

        set(run_cmd)
        if (CMAKE_HOST_WIN32)
            # Executing clang-format under windows runs a batch script that terminates the shell after running the first command
            # So in order to rung multiple clang-formats in one go, run it via cmd.exe, so that the custom target is not prematurely terminated
            set(run_cmd cmd /c)
        endif()

        foreach(clangformat_source ${non_generated_target_sources})
            list(APPEND clangformat_sources $<PATH:ABSOLUTE_PATH,${clangformat_source},${get_filename_component}>)

            string(LENGTH "${clangformat_sources}" size)
            if (${size} GREATER 7000)
                add_custom_command(
                    TARGET clangformat_${postfix} POST_BUILD
                    COMMAND ${run_cmd} ${CLANGFORMAT} -style=file:${MY_CLANG_FORMAT_FILE} -i ${clangformat_sources}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Formatting ${postfix} with ${CLANGFORMAT} ..."
                )
                set(clangformat_sources)
            endif()
        endforeach()

        if (clangformat_sources)
            add_custom_command(
                TARGET clangformat_${postfix} POST_BUILD
                COMMAND ${run_cmd} ${CLANGFORMAT} -style=file:${MY_CLANG_FORMAT_FILE} -i ${clangformat_sources}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Formatting ${postfix} with ${CLANGFORMAT} ..."
            )
        endif()
    else()
        add_custom_target(clangformat_${postfix}
            COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found; skipping formatting"
        )
    endif()
endfunction()
