add_executable(infra.stream_test)
emil_build_for(infra.stream_test BOOL EMIL_BUILD_TESTS)
emil_add_test(infra.stream_test)

target_link_libraries(infra.stream_test PUBLIC
    gmock_main
    infra.stream
    infra.util_test_helper
)

target_sources(infra.stream_test PRIVATE
    StreamMock.hpp
    TestAtomicByteQueue.cpp
    TestBoundedDequeInputStream.cpp
    TestBoundedVectorOutputStream.cpp
    TestBufferingStreamReader.cpp
    TestBufferingStreamWriter.cpp
    TestByteInputStream.cpp
    TestByteOutputStream.cpp
    TestCountingInputStream.cpp
    TestCountingOutputStream.cpp
    TestFormatter.cpp
    TestLimitedInputStream.cpp
    TestLimitedOutputStream.cpp
    TestOutputStreamSwitch.cpp
    TestStdStringInputStream.cpp
    TestStdStringOutputStream.cpp
    TestStdVectorInputStream.cpp
    TestStringInputStream.cpp
    TestStringOutputStream.cpp
)
