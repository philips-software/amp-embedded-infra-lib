#ifndef INFRA_STREAM_READER_MOCK
#define INFRA_STREAM_READER_MOCK

#include "infra/stream/InputStream.hpp"
#include "infra/stream/OutputStream.hpp"
#include "gmock/gmock.h"

namespace infra
{
    class StreamReaderMock
        : public infra::StreamReader
    {
    public:
        using infra::StreamReader::StreamReader;

        MOCK_METHOD2(Extract, void(ByteRange range, StreamErrorPolicy& errorPolicy));
        MOCK_METHOD1(Peek, uint8_t(StreamErrorPolicy& errorPolicy));
        MOCK_METHOD1(ExtractContiguousRange, infra::ConstByteRange(std::size_t max));
        MOCK_METHOD1(PeekContiguousRange, infra::ConstByteRange(std::size_t start));
        MOCK_CONST_METHOD0(Empty, bool());
        MOCK_CONST_METHOD0(Available, std::size_t());
    };

    class StreamReaderWithRewindingMock
        : public infra::StreamReaderWithRewinding
    {
    public:
        using infra::StreamReaderWithRewinding::StreamReaderWithRewinding;

        MOCK_METHOD2(Extract, void(ByteRange range, StreamErrorPolicy& errorPolicy));
        MOCK_METHOD1(Peek, uint8_t(StreamErrorPolicy& errorPolicy));
        MOCK_METHOD1(ExtractContiguousRange, infra::ConstByteRange(std::size_t max));
        MOCK_METHOD1(PeekContiguousRange, infra::ConstByteRange(std::size_t start));
        MOCK_CONST_METHOD0(Empty, bool());
        MOCK_CONST_METHOD0(Available, std::size_t());
        MOCK_CONST_METHOD0(ConstructSaveMarker, std::size_t());
        MOCK_METHOD1(Rewind, void(std::size_t marker));
    };

    class StreamWriterMock
        : public infra::StreamWriter
    {
    public:
        using infra::StreamWriter::StreamWriter;

        MOCK_METHOD2(Insert, void(ConstByteRange range, StreamErrorPolicy& errorPolicy));
        MOCK_CONST_METHOD0(Available, std::size_t());
        MOCK_CONST_METHOD0(ConstructSaveMarker, std::size_t());
        MOCK_CONST_METHOD1(GetProcessedBytesSince, std::size_t(std::size_t marker));
        MOCK_METHOD1(SaveState, infra::ByteRange(std::size_t marker));
        MOCK_METHOD1(RestoreState, void(infra::ByteRange range));
        MOCK_METHOD1(Overwrite, infra::ByteRange(std::size_t marker));
    };
}

#endif
