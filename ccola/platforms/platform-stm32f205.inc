set(CCOLA_CORE cortex-m3)
set(CCOLA_PROCESSOR_FAMILY stm32f2xx)
set(CCOLA_PROCESSOR stm32f205)

include(${CCOLA_DIR}/platforms/platform-cortex-m.inc)

add_definitions(-DSTM32F2 -DSTM32F205xx)
