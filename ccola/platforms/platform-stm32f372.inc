set(CCOLA_CORE cortex-m4)
set(CCOLA_PROCESSOR_FAMILY stm32f3xx)
set(CCOLA_PROCESSOR stm32f372)

include(${CCOLA_DIR}/platforms/platform-cortex-m.inc)

add_definitions(-DSTM32F3 -DSTM32F373xC)
