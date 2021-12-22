include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(hints_paths)

if(CMAKE_HOST_WIN32)
    set(X86_PROGRAM_FILES "ProgramFiles(x86)")
    list(APPEND hints_paths "$ENV{${X86_PROGRAM_FILES}}/GNU Tools Arm Embedded/8 2019-q3-update/bin/")
endif()

find_program(CMAKE_C_COMPILER
    NAMES arm-none-eabi-gcc
          arm-none-eabi-gcc.exe
    HINTS ${hints_paths})

find_program(CMAKE_CXX_COMPILER
    NAMES arm-none-eabi-g++
          arm-none-eabi-g++.exe
    HINTS ${hints_paths})

find_program(CMAKE_ASM_COMPILER
    NAMES arm-none-eabi-as
          arm-none-eabi-as.exe
    HINTS ${hints_paths})

find_program(CMAKE_AR
    NAMES arm-none-eabi-ar
          arm-none-eabi-ar.exe
    HINTS ${hints_paths})

find_program(CMAKE_RANLIB
    NAMES arm-none-eabi-ranlib
          arm-none-eabi-ranlib.exe
    HINTS ${hints_paths})

find_program(CMAKE_NM
    NAMES arm-none-eabi-nm
          arm-none-eabi-nm.exe
    HINTS ${hints_paths})

find_program(CCOLA_LD_TOOL
    NAMES arm-none-eabi-ld
          arm-none-eabi-ld.exe
    HINTS ${hints_paths})

find_program(CCOLA_APP_SIZE_TOOL
    NAMES arm-none-eabi-size
          arm-none-eabi-size.exe
    HINTS ${hints_paths})

find_program(CCOLA_OBJ_COPY_TOOL
    NAMES arm-none-eabi-objcopy
          arm-none-eabi-objcopy.exe
    HINTS ${hints_paths})

find_program(CCOLA_OBJ_DUMP_TOOL
    NAMES arm-none-eabi-objdump
          arm-none-eabi-objdump.exe
    HINTS ${hints_paths})

find_program(CCOLA_DEBUG_TOOL
    NAMES arm-none-eabi-gdb
          arm-none-eabi-gdb.exe
    HINTS ${hints_paths})
