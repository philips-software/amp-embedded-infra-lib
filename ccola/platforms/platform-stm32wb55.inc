set(CCOLA_CORE cortex-m4)
set(CCOLA_PROCESSOR_FAMILY stm32wbxx)

include(${CCOLA_DIR}/platforms/platform-cortex-m.inc)

set(combined_flags "-mfpu=fpv4-sp-d16 -mfloat-abi=softfp")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${combined_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${combined_flags}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mfpu=fpv4-sp-d16 -mfloat-abi=softfp")

add_definitions(-DSTM32WB -DSTM32WB55xx)
