include(CMakeForceCompiler)

find_program(gcc_path NAMES "arm-none-eabi-gcc.exe" PATHS "C:/Ac6/SystemWorkbench/plugins/*/tools/compiler/bin")
if(NOT gcc_path)
	message(FATAL_ERROR "GCC not found. Make sure you have started SystemWorkbench at least once to install the toolchain.")
endif()

get_filename_component(tools_path "${gcc_path}" DIRECTORY)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER "${tools_path}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${tools_path}/arm-none-eabi-g++.exe")

set(CMAKE_ASM_COMPILER "${tools_path}/arm-none-eabi-as.exe" CACHE FILEPATH as)
set(CMAKE_AR "${tools_path}/arm-none-eabi-gcc-ar.exe" CACHE FILEPATH ar)
set(CMAKE_RANLIB "${tools_path}/arm-none-eabi-gcc-ranlib.exe" CACHE FILEPATH ranlib)
set(CMAKE_NM "${tools_path}/arm-none-eabi-gcc-rm.exe" CACHE FILEPATH nm)

set(CCOLA_LD_TOOL "${tools_path}/arm-none-eabi-ld.exe")
set(CCOLA_APP_SIZE_TOOL "${tools_path}/arm-none-eabi-size.exe")
set(CCOLA_OBJ_COPY_TOOL "${tools_path}/arm-none-eabi-objcopy.exe")
set(CCOLA_OBJ_DUMP_TOOL "${tools_path}/arm-none-eabi-objdump.exe")
set(CCOLA_DEBUG_TOOL "${tools_path}/arm-none-eabi-gdb.exe")
