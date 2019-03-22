set(CCOLA_ARCH_FAMILY cortex)

set(combined_flags "-g -mthumb -ffunction-sections -fdata-sections -mcpu=${CCOLA_CORE}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${combined_flags} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${combined_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-use-cxa-atexit -fno-threadsafe-statics")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${combined_flags}" CACHE STRING CMAKE_ASM_FLAGS FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -g -mthumb -Wl,--gc-sections -nostartfiles -mcpu=${CCOLA_CORE}")

set(CMAKE_EXECUTABLE_SUFFIX .elf)
