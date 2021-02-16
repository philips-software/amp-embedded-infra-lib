ccola_component(synchronous_interfaces static_library)

    ccola_sources(
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

    ccola_dependencies(
        infra.util
    )

ccola_end_component()

ccola_subdirectories(
    test_doubles
)
