#ifndef UPGRADE_BINARY_OBJECT_HPP
#define UPGRADE_BINARY_OBJECT_HPP

#include "upgrade/pack_builder/Elf.hpp"
#include "upgrade/pack_builder/SparseVector.hpp"
#include <string>

namespace application
{
    class LineException
        : public std::runtime_error
    {
    public:
        LineException(const std::string& prependMessage, const std::string& file, int line);
    };

    class IncorrectCrcException
        : public LineException
    {
    public:
        IncorrectCrcException(const std::string& file, int line);
    };

    class NoEndOfFileException
        : public LineException
    {
    public:
        NoEndOfFileException(const std::string& file, int line);
    };

    class DataAfterEndOfFileException
        : public LineException
    {
    public:
        DataAfterEndOfFileException(const std::string& file, int line);
    };

    class UnknownRecordException
        : public LineException
    {
    public:
        UnknownRecordException(const std::string& file, int line);
    };

    class RecordTooShortException
        : public LineException
    {
    public:
        RecordTooShortException(const std::string& file, int line);
    };

    class RecordTooLongException
        : public LineException
    {
    public:
        RecordTooLongException(const std::string& file, int line);
    };

    class BinaryObject
    {
    public:
        void AddHex(const std::vector<std::string>& data, uint32_t offset, const std::string& fileName);
        void AddElf(const std::vector<uint8_t>& data, uint32_t offset, const std::string& fileName);
        void AddBinary(const std::vector<uint8_t>& data, uint32_t offset, const std::string& fileName);

        const SparseVector<uint8_t>& Memory() const;

    private:
        struct LineContents
        {
            LineContents(const std::string& line, const std::string& fileName, int lineNumber);

            uint8_t recordType;
            uint16_t address;
            std::vector<uint8_t> data;
        };

    private:
        std::string SectionName(const std::vector<uint8_t>& data, const uint32_t sectionNameOffset);
        void AddLine(const std::string& line, const std::string& fileName, int lineNumber);
        void VerifyNotEndOfFile(const std::string& fileName, int lineNumber) const;
        void InsertLineContents(const LineContents& lineContents);

    private:
        SparseVector<uint8_t> memory;
        bool endOfFile = false;
        uint32_t linearAddress = 0;
        uint32_t offset = 0;
    };
}

#endif
