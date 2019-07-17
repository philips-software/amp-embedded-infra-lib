#ifndef UPGRADE_BINARY_OBJECT_HPP
#define UPGRADE_BINARY_OBJECT_HPP

#include <string>
#include "upgrade/pack_builder/SparseVector.hpp"
#include "upgrade/pack_builder/Elf.hpp"

namespace application
{
    struct LineException
    {
        LineException(const std::string& file, int line);

        std::string file;
        int line;
    };

    struct IncorrectCrcException
        : LineException
    {
        IncorrectCrcException(const std::string& file, int line);
    };

    struct NoEndOfFileException
        : LineException
    {
        NoEndOfFileException(const std::string& file, int line);
    };

    struct DataAfterEndOfFileException
        : LineException
    {
        DataAfterEndOfFileException(const std::string& file, int line);
    };

    struct UnknownRecordException
        : LineException
    {
        UnknownRecordException(const std::string& file, int line);
    };

    struct RecordTooShortException
        : LineException
    {
        RecordTooShortException(const std::string& file, int line);
    };

    struct RecordTooLongException
        : LineException
    {
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
            LineContents(std::string line, const std::string& fileName, int lineNumber);

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
