set(CCOLA_CORE cortex-m0)
set(CCOLA_PROCESSOR_FAMILY stm32f0xx)
set(CCOLA_PROCESSOR stm32f091)

include(${CCOLA_DIR}/platforms/platform-cortex-m.inc)

add_definitions(-DSTM32F0 -DSTM32F091xC)
