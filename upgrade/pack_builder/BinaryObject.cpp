#include "upgrade/pack_builder/BinaryObject.hpp"
#include "infra/stream/StdStringInputStream.hpp"

namespace application
{
    LineException::LineException(const std::string& prependMessage, const std::string& file, int line)
        : runtime_error(prependMessage + " in file " + file + " at line " + std::to_string(line))
    {}

    IncorrectCrcException::IncorrectCrcException(const std::string& file, int line)
        : LineException("Incorrect CRC", file, line)
    {}

    NoEndOfFileException::NoEndOfFileException(const std::string& file, int line)
        : LineException("No end of file found", file, line)
    {}

    DataAfterEndOfFileException::DataAfterEndOfFileException(const std::string& file, int line)
        : LineException("Data found after end of file", file, line)
    {}

    UnknownRecordException::UnknownRecordException(const std::string& file, int line)
        : LineException("Unknown record", file, line)
    {}

    RecordTooShortException::RecordTooShortException(const std::string& file, int line)
        : LineException("Record too short", file, line)
    {}

    RecordTooLongException::RecordTooLongException(const std::string& file, int line)
        : LineException("Record too long", file, line)
    {}

    void BinaryObject::AddHex(const std::vector<std::string>& data, uint32_t offset, const std::string& fileName)
    {
        linearAddress = 0;
        endOfFile = false;
        this->offset = offset;

        int lineNumber = 0;
        for (auto line : data)
        {
            ++lineNumber;
            if (!line.empty())
                AddLine(line, fileName, lineNumber);
        }

        if (!endOfFile)
            throw NoEndOfFileException(fileName, lineNumber);
    }

    void BinaryObject::AddBinary(const std::vector<uint8_t>& data, uint32_t offset, const std::string& fileName)
    {
        for (auto byte: data)
        {
            memory.Insert(byte, offset);
            ++offset;
        }
    }

    const SparseVector<uint8_t>& BinaryObject::Memory() const
    {
        return memory;
    }

    void BinaryObject::AddLine(const std::string& line, const std::string& fileName, int lineNumber)
    {
        VerifyNotEndOfFile(fileName, lineNumber);

        LineContents lineContents(line, fileName, lineNumber);
        switch (lineContents.recordType)
        {
            case 0:
                InsertLineContents(lineContents);
                break;
            case 1:
                endOfFile = true;
                break;
            case 2:
                linearAddress = (lineContents.data[0] * 256 + lineContents.data[1]) << 4;
                break;
            case 3:
                // Ignore Start Segment Address because in hex file, the entrypoint of the program is not interesting
                break;
            case 4:
                assert(lineContents.data.size() == 2);
                linearAddress = (lineContents.data[0] * 256 + lineContents.data[1]) << 16;
                break;
            case 5:
                // Ignore Start Linear Address because in hex file, the entrypoint of the program is not interesting
                break;
            default:
                throw UnknownRecordException(fileName, lineNumber);
        }
    }
    
    void BinaryObject::VerifyNotEndOfFile(const std::string& fileName, int lineNumber) const
    {
        if (endOfFile)
            throw DataAfterEndOfFileException(fileName, lineNumber);
    }

    void BinaryObject::InsertLineContents(const LineContents& lineContents)
    {
        for (std::size_t i = 0; i != lineContents.data.size(); ++i)
            memory.Insert(lineContents.data[i], linearAddress + offset + lineContents.address + i);
    }

    BinaryObject::LineContents::LineContents(std::string line, const std::string& fileName, int lineNumber)
    {
        infra::StdStringInputStream stream(line, infra::softFail);

        char colon;
        stream >> colon;

        uint8_t size = 0;
        stream >> infra::hex >> infra::Width(2, '0') >> size >> infra::Width(4, '0') >> address >> infra::Width(2, '0') >> recordType;
        uint8_t sum = static_cast<uint8_t>(size + static_cast<uint8_t>(address) + static_cast<uint8_t>(address >> 8) + recordType);         //TICS !POR#006

        if (stream.Failed())
            throw RecordTooShortException(fileName, lineNumber);

        for (std::size_t i = 0; i != size; ++i)
        {
            uint8_t byte = 0;
            stream >> infra::hex >> infra::Width(2) >> byte;
            sum += byte;
            data.push_back(byte);
        }

        uint8_t checksum = 0;
        stream >> infra::hex >> infra::Width(2) >> checksum;
        sum += checksum;

        if (stream.Failed())
            throw RecordTooShortException(fileName, lineNumber);
        if (sum != 0)
            throw IncorrectCrcException(fileName, lineNumber);
        if (!stream.Empty())
            throw RecordTooLongException(fileName, lineNumber);
    }
}
