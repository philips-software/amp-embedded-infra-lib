add_executable(infra.stream_test)
add_test(NAME infra.stream_test COMMAND infra.stream_test)

target_link_libraries(infra.stream_test
    PUBLIC
    infra.stream
    infra.util_test_helper

    gtest
    gmock
    gmock_main
)

target_sources(infra.stream_test
    PRIVATE
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
