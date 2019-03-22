set(CCOLA_ARCH cortex-r)

set(combined_flags "-mthumb-interwork -fno-builtin-memcpy -mlittle-endian -fno-common")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${combined_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${combined_flags}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${combined_flags}" CACHE STRING CMAKE_ASM_FLAGS FORCE)

set(CMAKE_EXECUTABLE_SUFFIX .elf)

include(${CCOLA_DIR}/platforms/platform-cortex.inc)
