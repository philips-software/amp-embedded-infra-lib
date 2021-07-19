set(CCOLA_CORE cortex-m4)
set(CCOLA_PROCESSOR_FAMILY nrf528xx)
add_definitions(-DNRF528xx)

include(${CCOLA_DIR}/platforms/platform-cortex-m.inc)

set(combined_flags "-mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${combined_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${combined_flags} -Wno-register")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${combined_flags}")
