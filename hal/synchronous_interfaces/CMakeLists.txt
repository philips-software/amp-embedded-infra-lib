add_library(hal.synchronous_interfaces ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_include_directories(hal.synchronous_interfaces PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(hal.synchronous_interfaces PUBLIC
    infra.util
)

target_sources(hal.synchronous_interfaces PRIVATE
    SynchronousEncryptor.hpp
    SynchronousFlash.cpp
    SynchronousFlash.hpp
    SynchronousFlashHomogeneous.cpp
    SynchronousFlashHomogeneous.hpp
    SynchronousFlashId.hpp
    SynchronousGpio.hpp
    SynchronousI2c.hpp
    SynchronousQuadSpi.hpp
    SynchronousRandomDataGenerator.hpp
    SynchronousSerialCommunication.hpp
    SynchronousSpi.cpp
    SynchronousSpi.hpp
    TimeKeeper.hpp
    TimeService.hpp
)

if (BUILD_TESTING)
    add_subdirectory(test_doubles)
endif()
