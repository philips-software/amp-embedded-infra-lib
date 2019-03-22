set(CCOLA_CORE cortex-m7)
set(CCOLA_PROCESSOR_FAMILY stm32f7xx)
set(CCOLA_PROCESSOR stm32f767)

include(${CCOLA_DIR}/platforms/platform-cortex-m.inc)

set(combined_flags "-mfpu=fpv5-d16 -mfloat-abi=softfp")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${combined_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${combined_flags}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mfpu=fpv5-d16 -mfloat-abi=softfp")

add_definitions(-DSTM32F7 -DSTM32F767xx)
