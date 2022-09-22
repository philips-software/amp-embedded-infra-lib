find_program(DOCKER docker)

if (DOCKER)
    execute_process(
        COMMAND ${DOCKER} inspect -f "{{ range .Mounts }}{{ if eq .Destination \"/workspaces\" }}{{ .Name }}{{ end }}{{ end }}" $ENV{HOSTNAME}
        OUTPUT_VARIABLE docker_volume
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (NOT docker_volume STREQUAL "")
        set(docker_mountpoint ${docker_volume})
        set(docker_mountpoint_parent ${docker_volume})
        set(workspace_path $ENV{LOCAL_WORKSPACE_FOLDER})
        cmake_path(GET workspace_path STEM docker_mountpoint_stem)
        unset(workspace_path)
    elseif(NOT $ENV{LOCAL_WORKSPACE_FOLDER} STREQUAL "")
        string(REPLACE "\\" "/" docker_mountpoint $ENV{LOCAL_WORKSPACE_FOLDER})
        cmake_path(GET docker_mountpoint PARENT_PATH docker_mountpoint_parent)
        cmake_path(GET docker_mountpoint STEM docker_mountpoint_stem)
    else()
        message(WARNING "Could not determine paths and mount-points for Docker tools")
    endif()

    add_custom_target(lint
        COMMAND ${DOCKER} run --rm -v ${docker_mountpoint_parent}:/workspaces -e DEFAULT_WORKSPACE=/workspaces/${docker_mountpoint_stem} -e APPLY_FIXES=all oxsecurity/megalinter:v6
        COMMENT "Running linter..."
        USES_TERMINAL
        VERBATIM
    )
else()
    message(WARNING "Inclusion of Docker tools requested, but Docker could not be found. Skipped adding Docker targets.")
endif()
