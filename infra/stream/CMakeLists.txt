add_library(infra.stream ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(infra.stream PUBLIC
    infra.util
)

target_sources(infra.stream PRIVATE
    AtomicByteQueue.cpp
    AtomicByteQueue.hpp
    BoundedDequeInputStream.cpp
    BoundedDequeInputStream.hpp
    BoundedDequeOutputStream.cpp
    BoundedDequeOutputStream.hpp
    BoundedVectorInputStream.cpp
    BoundedVectorInputStream.hpp
    BoundedVectorOutputStream.cpp
    BoundedVectorOutputStream.hpp
    BufferingStreamReader.cpp
    BufferingStreamReader.hpp
    BufferingStreamWriter.cpp
    BufferingStreamWriter.hpp
    ByteInputStream.cpp
    ByteInputStream.hpp
    ByteOutputStream.cpp
    ByteOutputStream.hpp
    ConstructBinStream.cpp
    ConstructBinStream.hpp
    CountingInputStream.cpp
    CountingInputStream.hpp
    CountingOutputStream.cpp
    CountingOutputStream.hpp
    Formatter.cpp
    Formatter.hpp
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
    StdVectorInputStream.cpp
    StdVectorInputStream.hpp
    StdVectorOutputStream.cpp
    StdVectorOutputStream.hpp
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

if (EMIL_HOST_BUILD)
    target_sources(infra.stream PRIVATE
        StreamWriterFile.cpp
        StreamWriterFile.hpp
    )
endif()

add_subdirectory(test)
