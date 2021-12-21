add_executable(infra.stream_test)
gtest_discover_tests(infra.stream_test AUTO)

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
