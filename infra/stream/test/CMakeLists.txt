add_executable(infra.stream_test)
emil_add_test(infra.stream_test)

target_link_libraries(infra.stream_test PUBLIC
    gmock_main
    infra.stream
    infra.util_test_helper
)

target_sources(infra.stream_test PRIVATE
    StreamMock.hpp
    TestBoundedDequeInputStream.cpp
    TestBoundedVectorOutputStream.cpp
    TestByteInputStream.cpp
    TestByteOutputStream.cpp
    TestCountingInputStream.cpp
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
