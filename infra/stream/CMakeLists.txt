ccola_component(stream static_library)

    ccola_sources(
        BoundedDequeInputStream.cpp
        BoundedDequeInputStream.hpp
        ByteInputStream.cpp
        ByteInputStream.hpp
        ByteOutputStream.cpp
        ByteOutputStream.hpp
        CountingInputStream.cpp
        CountingInputStream.hpp
        InputStream.cpp
        InputStream.hpp
        IoOutputStream.cpp
        IoOutputStream.hpp
        LimitedInputStream.cpp
        LimitedInputStream.hpp
        LimitedOutputStream.cpp
        LimitedOutputStream.hpp
        OutputStream.cpp
        OutputStream.hpp
        OverwriteStream.cpp
        OverwriteStream.hpp
        SavedMarkerStream.cpp
        SavedMarkerStream.hpp
        StdStringInputStream.cpp
        StdStringInputStream.hpp
        StdStringOutputStream.cpp
        StdStringOutputStream.hpp
        StreamErrorPolicy.cpp
        StreamErrorPolicy.hpp
        StreamManipulators.cpp
        StreamManipulators.hpp
        StreamWriterPrintF.cpp
        StreamWriterPrintF.hpp
        StringInputStream.cpp
        StringInputStream.hpp
        StringOutputStream.cpp
        StringOutputStream.hpp
    )

    ccola_dependencies(
        util
    )

ccola_end_component()

ccola_subdirectories(
    test
)
