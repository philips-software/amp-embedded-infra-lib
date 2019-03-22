# Remove any .launch file that is left over from a previous CMake run, from a target that is now removed or renamed
file(STRINGS "${CCOLA_BUILD_DIR}/eclipse_launch_files.txt" configured_launch_files)

file(GLOB existing_launch_files RELATIVE ${CCOLA_BUILD_DIR} ${CCOLA_BUILD_DIR}/*.launch)

foreach(existing_launch_file ${existing_launch_files})
	list(FIND configured_launch_files "${existing_launch_file}" index)
	if (index STREQUAL -1)
		file(REMOVE ${CCOLA_BUILD_DIR}/${existing_launch_file})
	endif()
endforeach()

file(REMOVE "${CCOLA_BUILD_DIR}/eclipse_launch_files.txt")
