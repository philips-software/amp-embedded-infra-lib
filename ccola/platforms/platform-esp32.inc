set(CCOLA_CORE xtensa-lx6)
set(CCOLA_PROCESSOR_FAMILY xtensa)
set(CCOLA_PROCESSOR xtensa-lx6)
set(CCOLA_ARCH xtensa)
set(CCOLA_ARCH_FAMILY xtensa)

set(combined_flags "-g -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${combined_flags} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${combined_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-use-cxa-atexit -fno-threadsafe-statics -mlongcalls -mtext-section-literals")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${combined_flags}" CACHE STRING CMAKE_ASM_FLAGS FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -u call_user_start_cpu0 -Wl,--gc-sections -Wl,-static -Wl,-EL")

set(CMAKE_EXECUTABLE_SUFFIX .elf)
