add_library(infra.stream EXCLUDE_FROM_ALL STATIC)
install(TARGETS infra.stream EXPORT emilInfraTargets)

target_link_libraries(infra.stream PUBLIC
    infra.util
)

target_sources(infra.stream PRIVATE
    BoundedDequeInputStream.cpp
    BoundedDequeInputStream.hpp
    BoundedDequeOutputStream.cpp
    BoundedDequeOutputStream.hpp
    BoundedVectorInputStream.cpp
    BoundedVectorInputStream.hpp
    BoundedVectorOutputStream.cpp
    BoundedVectorOutputStream.hpp
    ByteInputStream.cpp
    ByteInputStream.hpp
    ByteOutputStream.cpp
    ByteOutputStream.hpp
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
    StreamWriterFile.cpp
    StreamWriterFile.hpp
    StreamWriterPrintF.cpp
    StreamWriterPrintF.hpp
    StringInputStream.cpp
    StringInputStream.hpp
    StringOutputStream.cpp
    StringOutputStream.hpp
)

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
