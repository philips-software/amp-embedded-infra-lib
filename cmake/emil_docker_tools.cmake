# This module will include Docker tools as custom targets in a build-tree.
#
# Three scenario's are supported:
# - Running inside a devcontainer using a volume-mount (e.g. clone in container volume)
# - Running inside a devcontainer using a bind-mount (e.g. (re-)open in container)
# - Running native with Docker installed on the host
#
# The first two scenarios require a Docker cli to be installed inside the container,
# the Docker socket exposed inside that container and the workspace location exported
# as environment variable (LOCAL_WORKSPACE_FOLDER).
# See: https://github.com/microsoft/vscode-dev-containers/tree/main/containers/docker-from-docker
#
# The third scenario directly interacts with the Docker instance on the host machine.

function(emil_enable_docker_tools)
    find_program(DOCKER docker)

    if (DOCKER)
        cmake_host_system_information(RESULT hostname QUERY HOSTNAME)

        execute_process(
            COMMAND ${DOCKER} inspect -f "{{ range .Mounts }}{{ if eq .Destination \"/workspaces\" }}{{ .Name }}{{ end }}{{ end }}" ${hostname}
            OUTPUT_VARIABLE docker_volume
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
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
            string(REPLACE "\\" "/" docker_mountpoint ${CMAKE_SOURCE_DIR})
            cmake_path(GET docker_mountpoint PARENT_PATH docker_mountpoint_parent)
            cmake_path(GET docker_mountpoint STEM docker_mountpoint_stem)
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
endfunction()
