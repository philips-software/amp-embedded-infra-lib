set(CCOLA_CORE cortex-m4)
set(CCOLA_PROCESSOR_FAMILY nrf528xx)
set(CCOLA_PROCESSOR nrf52833)

include(${CCOLA_DIR}/platforms/platform-cortex-m.inc)

set(combined_flags "-mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${combined_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${combined_flags}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

add_definitions(-DNRF528xx -DNRF52833)
