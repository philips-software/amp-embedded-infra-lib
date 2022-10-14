#include "gtest/gtest.h"
#include "infra/util/BoundedString.hpp"
#include "infra/util/ByteRange.hpp"

TEST(BoundedStringTest, TestConstructedEmpty)
{
    infra::BoundedString::WithStorage<5> string;

    EXPECT_TRUE(string.empty());
    EXPECT_FALSE(string.full());
    EXPECT_EQ(0, string.size());
    EXPECT_EQ(5, string.max_size());
}

TEST(BoundedStringTest, TestConstructFromMemoryRange)
{
    std::array<char, 5> data = { 'a', 'b', 'c', 'd', 'e' };
    infra::BoundedString string(infra::MakeRange(data), 3);

    EXPECT_EQ('a', string[0]);
    EXPECT_EQ('b', string[1]);
    EXPECT_EQ('c', string[2]);
    EXPECT_EQ(3, string.size());
    EXPECT_EQ(5, string.max_size());
    EXPECT_FALSE(string.empty());
}

TEST(BoundedStringTest, TestConstructFromNChars)
{
    infra::BoundedString::WithStorage<5> string(3, 'a');

    EXPECT_EQ('a', string[0]);
    EXPECT_EQ('a', string[1]);
    EXPECT_EQ('a', static_cast<const infra::BoundedString&>(string)[2]);
    EXPECT_EQ(3, string.size());
    EXPECT_FALSE(string.empty());
}

TEST(BoundedStringTest, TestConstructFromOtherString)
{
    infra::BoundedString::WithStorage<5> string(3, 'a');

    infra::BoundedString::WithStorage<4> copy1(string);
    EXPECT_EQ('a', copy1[0]);
    EXPECT_EQ(3, copy1.size());

    infra::BoundedString::WithStorage<4> copy2(string, 0, 2);
    EXPECT_EQ('a', copy2[0]);
    EXPECT_EQ(2, copy2.size());
}

TEST(BoundedStringTest, TestConstructFromStdString)
{
    std::string string(3, 'a');
    infra::BoundedString::WithStorage<4> copy(string);

    EXPECT_EQ('a', copy[0]);
    EXPECT_EQ(3, copy.size());
}

TEST(BoundedStringTest, TestConstructFromStdStringAsStorage)
{
    std::string string(3, 'a');
    infra::BoundedString copy(string);

    EXPECT_EQ('a', copy[0]);
    EXPECT_EQ(3, copy.size());
}

TEST(BoundedStringTest, TestConstructFromConstStdStringAsStorage)
{
    const std::string string(3, 'a');
    infra::BoundedConstString copy(string);

    EXPECT_EQ('a', copy[0]);
    EXPECT_EQ(3, copy.size());
}

TEST(BoundedStringTest, TestConstructFromCharP)
{
    char a[] = { 'a', '\0', 'a' };
    infra::BoundedString::WithStorage<3> string(a, 3);

    EXPECT_EQ('a', string[0]);
    EXPECT_EQ('\0', string[1]);
    EXPECT_EQ('a', string[2]);
    EXPECT_EQ(3, string.size());
}

TEST(BoundedStringTest, TestConstructFromZeroTerminatedString)
{
    char a[] = { 'a', '\0', 'a' };
    infra::BoundedString::WithStorage<3> string(a);

    EXPECT_EQ('a', string[0]);
    EXPECT_EQ(1, string.size());
}

TEST(BoundedStringTest, TestConstructFromRange)
{
    char a[] = { 'a', '\0', 'a' };
    infra::BoundedString::WithStorage<3> string(a, a + 3);

    EXPECT_EQ('a', string[0]);
    EXPECT_EQ('\0', string[1]);
    EXPECT_EQ('a', string[2]);
    EXPECT_EQ(3, string.size());
}

TEST(BoundedStringTest, TestConstructFromInitializerList)
{
    infra::BoundedString::WithStorage<3> string({ 'a', '\0', 'a' });

    EXPECT_EQ('a', string[0]);
    EXPECT_EQ('\0', string[1]);
    EXPECT_EQ('a', string[2]);
    EXPECT_EQ(3, string.size());
}

TEST(BoundedStringTest, TestCopyAssignFromString)
{
    infra::BoundedString::WithStorage<5> string(3, 'a');
    infra::BoundedString::WithStorage<5> copy;

    copy = string;

    EXPECT_EQ(string, copy);
}

TEST(BoundedStringTest, TestCopyAssignFromStdString)
{
    std::string string(3, 'a');
    infra::BoundedString::WithStorage<5> copy;

    copy = string;

    EXPECT_EQ(string, copy);
}

TEST(BoundedStringTest, TestCopyAssignFromZeroTerminated)
{
    infra::BoundedString::WithStorage<5> string;
    string = "abc";

    EXPECT_EQ("abc", string);
    EXPECT_EQ(3, string.size());
}

TEST(BoundedStringTest, TestCopyAssignFromChar)
{
    infra::BoundedString::WithStorage<5> string;
    string = 'a';

    EXPECT_EQ("a", string);
}

TEST(BoundedStringTest, TestAssignFromString)
{
    infra::BoundedString::WithStorage<5> string(3, 'a');
    infra::BoundedString::WithStorage<5> copy;

    copy.assign(string);

    EXPECT_EQ(string, copy);
}

TEST(BoundedStringTest, TestAssignFromStdString)
{
    std::string string(3, 'a');
    infra::BoundedString::WithStorage<5> copy;

    copy.assign(string);

    EXPECT_EQ(string, copy);
}

TEST(BoundedStringTest, TestAssignFromZeroTerminated)
{
    infra::BoundedString::WithStorage<5> string;
    string.assign("abc");

    EXPECT_EQ("abc", string);
    EXPECT_EQ(3, string.size());
}

TEST(BoundedStringTest, TestAssignFromChar)
{
    infra::BoundedString::WithStorage<5> string;
    string.assign(1, 'a');

    EXPECT_EQ("a", string);
}

TEST(BoundedStringTest, TestFront)
{
    infra::BoundedString::WithStorage<5> string("abc");
    EXPECT_EQ('a', string.front());
    EXPECT_EQ('a', static_cast<const infra::BoundedString&>(string).front());
}

TEST(BoundedStringTest, TestBack)
{
    infra::BoundedString::WithStorage<5> string("abc");
    EXPECT_EQ('c', string.back());
    EXPECT_EQ('c', static_cast<const infra::BoundedString&>(string).back());
}

TEST(BoundedStringTest, TestData)
{
    infra::BoundedString::WithStorage<5> string("abc");
    EXPECT_EQ('a', *string.data());
}

TEST(BoundedStringTest, TestBegin)
{
    infra::BoundedString::WithStorage<5> string("abc");
    EXPECT_EQ('a', *string.begin());
    EXPECT_EQ('a', *string.cbegin());
}

TEST(BoundedStringTest, TestEnd)
{
    infra::BoundedString::WithStorage<5> string("abc");
    EXPECT_EQ('c', *std::prev(string.end()));
    EXPECT_EQ('c', *std::prev(string.cend()));
}

TEST(BoundedStringTest, TestRBegin)
{
    infra::BoundedString::WithStorage<5> string("abc");
    EXPECT_EQ('c', *string.rbegin());
    EXPECT_EQ('c', *static_cast<const infra::BoundedString&>(string).rbegin());
    EXPECT_EQ('c', *string.crbegin());
}

TEST(BoundedStringTest, TestREnd)
{
    infra::BoundedString::WithStorage<5> string("abc");
    EXPECT_EQ('a', *std::prev(string.rend()));
    EXPECT_EQ('a', *std::prev(static_cast<const infra::BoundedString&>(string).rend()));
    EXPECT_EQ('a', *std::prev(string.crend()));
}

TEST(BoundedStringTest, TestClear)
{
    infra::BoundedString::WithStorage<5> string("abc");
    string.clear();
    EXPECT_EQ("", string);
    EXPECT_TRUE(string.empty());
}

TEST(BoundedStringTest, TestAppendWithChars)
{
    infra::BoundedString::WithStorage<10> string("abc");
    string.append(2, 'd');
    EXPECT_EQ("abcdd", string);
    EXPECT_EQ(5, string.size());
    string += 'e';
    EXPECT_EQ("abcdde", string);
}

TEST(BoundedStringTest, TestAppendWithString)
{
    infra::BoundedString::WithStorage<10> string("abc");
    string += infra::BoundedConstString("de");
    EXPECT_EQ("abcde", string);
    EXPECT_EQ(5, string.size());
    string += "fg";
    EXPECT_EQ("abcdefg", string);
    string += std::string("hi");
    EXPECT_EQ("abcdefghi", string);
}

TEST(BoundedStringTest, TestAppendWithSubString)
{
    infra::BoundedString::WithStorage<5> string("abc");
    string.append(infra::BoundedString::WithStorage<5>("defgh"), 1, 2);
    EXPECT_EQ("abcef", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestAppendWithCharP)
{
    infra::BoundedString::WithStorage<6> string("abc");
    string.append("d\0ef", 3);
    EXPECT_EQ(infra::BoundedString::WithStorage<6>("abcd\0e", 6), string);
    EXPECT_EQ(6, string.size());
}

TEST(BoundedStringTest, TestAppendWithZeroTerminated)
{
    infra::BoundedString::WithStorage<5> string("abc");
    string.append("d\0ef");
    EXPECT_EQ("abcd", string);
    EXPECT_EQ(4, string.size());
}

TEST(BoundedStringTest, TestAppendWithRange)
{
    char a[] = "def";
    infra::BoundedString::WithStorage<5> string("abc");
    string.append(a, a + 3);
    EXPECT_EQ("abcde", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestInsertChars)
{
    infra::BoundedString::WithStorage<5> string("abc");
    string.insert(1, 2, 'd');

    EXPECT_EQ("addbc", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestInsertZeroTerminated)
{
    infra::BoundedString::WithStorage<5> string("abc");
    string.insert(1, "de");

    EXPECT_EQ("adebc", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestInsertCharP)
{
    infra::BoundedString::WithStorage<6> string("abc");
    string.insert(1, "d\0ef", 3);
    EXPECT_EQ(infra::BoundedString::WithStorage<6>("ad\0ebc", 6), string);
    EXPECT_EQ(6, string.size());
}

TEST(BoundedStringTest, TestInsertString)
{
    infra::BoundedString::WithStorage<20> string("abc");
    infra::BoundedString::WithStorage<3> other("de");
    string.insert(1, other);
    EXPECT_EQ("adebc", string);
    EXPECT_EQ(5, string.size());
    string.insert(1, other, 0, 1);
    EXPECT_EQ("addebc", string);
    string.insert(1, std::string("x"));
    EXPECT_EQ("axddebc", string);
    string.insert(1, std::string("x"), 0, 1);
    EXPECT_EQ("axxddebc", string);
}

TEST(BoundedStringTest, TestInsertCharAtIterator)
{
    infra::BoundedString::WithStorage<5> string("abc");
    infra::BoundedString::WithStorage<5>::const_iterator i = string.insert(string.begin() + 1, 'd');
    EXPECT_EQ(string.begin() + 2, i);
    EXPECT_EQ("adbc", string);
    EXPECT_EQ(4, string.size());
}

TEST(BoundedStringTest, TestInsertCharAtIteratorWithEndAsReturn)
{
    infra::BoundedString::WithStorage<5> string("abcd");
    infra::BoundedString::WithStorage<5>::const_iterator i = string.insert(string.begin() + 4, 'e');
    EXPECT_EQ(string.end(), i);
    EXPECT_EQ("abcde", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestInsertCharsAtIterator)
{
    infra::BoundedString::WithStorage<5> string("abc");
    infra::BoundedString::WithStorage<5>::const_iterator i = string.insert(string.begin() + 1, 2, 'd');
    EXPECT_EQ(string.begin() + 3, i);
    EXPECT_EQ("addbc", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestInsertCharsAtIteratorWithEndAsReturn)
{
    infra::BoundedString::WithStorage<5> string("abc");
    infra::BoundedString::WithStorage<5>::const_iterator i = string.insert(string.begin() + 3, 2, 'd');
    EXPECT_EQ(string.end(), i);
    EXPECT_EQ("abcdd", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestInsertRangeAtIterator)
{
    char a[] = "de";
    infra::BoundedString::WithStorage<5> string("abc");
    infra::BoundedString::WithStorage<5>::const_iterator i = string.insert(string.begin() + 1, a, a + sizeof(a));
    EXPECT_EQ(string.begin() + 3, i);
    EXPECT_EQ("adebc", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestEraseCount)
{
    infra::BoundedString::WithStorage<5> string("abcd");
    string.erase(1, 2);
    EXPECT_EQ("ad", string);
    EXPECT_EQ(2, string.size());
}

TEST(BoundedStringTest, TestEraseChar)
{
    infra::BoundedString::WithStorage<5> string("abcd");
    infra::BoundedString::WithStorage<5>::const_iterator i = string.erase(string.begin() + 1);
    EXPECT_EQ('c', *i);
    EXPECT_EQ("acd", string);
    EXPECT_EQ(3, string.size());
}

TEST(BoundedStringTest, TestEraseRange)
{
    infra::BoundedString::WithStorage<5> string("abcd");
    infra::BoundedString::WithStorage<5>::const_iterator i = string.erase(string.begin() + 1, string.begin() + 3);
    EXPECT_EQ('d', *i);
    EXPECT_EQ("ad", string);
    EXPECT_EQ(2, string.size());
}

TEST(BoundedStringTest, TestPushBack)
{
    infra::BoundedString::WithStorage<5> string("abcd");
    string.push_back('e');
    EXPECT_EQ("abcde", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestPopBack)
{
    infra::BoundedString::WithStorage<5> string("abcd");
    string.pop_back();
    EXPECT_EQ("abc", string);
    EXPECT_EQ(3, string.size());
}

TEST(BoundedStringTest, TestCompareEqual)
{
    infra::BoundedString::WithStorage<5> string1("abcd");
    infra::BoundedString::WithStorage<10> string2("abcd");
    EXPECT_EQ(0, string1.compare(string2));
    EXPECT_EQ(0, string1.compare(0, 4, string2));
    EXPECT_EQ(0, string1.compare(0, 4, string2, 0, 4));
    EXPECT_EQ(0, string1.compare("abcd"));
    EXPECT_EQ(0, string1.compare(0, 4, "abcd"));
    EXPECT_EQ(0, string1.compare(0, 4, "abcd", 4));
    EXPECT_EQ(0, string1.compare(std::string("abcd")));
    EXPECT_EQ(0, string1.compare(0, 4, std::string("abcd")));
    EXPECT_EQ(0, string1.compare(0, 4, std::string("abcd"), 4));
}

TEST(BoundedStringTest, TestCompareLess)
{
    infra::BoundedString::WithStorage<5> string1("abbb");
    infra::BoundedString::WithStorage<10> string2("abcd");
    EXPECT_EQ(-1, string1.compare(string2));
}

TEST(BoundedStringTest, TestCompareLessBySize)
{
    infra::BoundedString::WithStorage<5> string1("abc");
    infra::BoundedString::WithStorage<10> string2("abcd");
    EXPECT_EQ(-1, string1.compare(string2));
}

TEST(BoundedStringTest, TestCompareGreater)
{
    infra::BoundedString::WithStorage<5> string1("abde");
    infra::BoundedString::WithStorage<10> string2("abcd");
    EXPECT_EQ(1, string1.compare(string2));
}

TEST(BoundedStringTest, TestCompareGreaterBySize)
{
    infra::BoundedString::WithStorage<5> string1("abcde");
    infra::BoundedString::WithStorage<10> string2("abcd");
    EXPECT_EQ(1, string1.compare(string2));
}

TEST(BoundedStringTest, TestCompareOperators)
{
    infra::BoundedConstString string("abc");
    char stringC[] = "abc";
    std::string stringStd = "abc";

    EXPECT_TRUE(string == string);
    EXPECT_TRUE(string == stringC);
    EXPECT_TRUE(stringC == string);
    EXPECT_TRUE(string == stringStd);
    EXPECT_TRUE(stringStd == string);

    EXPECT_FALSE(string != string);
    EXPECT_FALSE(string != stringC);
    EXPECT_FALSE(stringC != string);
    EXPECT_FALSE(string != stringStd);
    EXPECT_FALSE(stringStd != string);

    EXPECT_FALSE(string < string);
    EXPECT_FALSE(string < stringC);
    EXPECT_FALSE(stringC < string);
    EXPECT_FALSE(string < stringStd);
    EXPECT_FALSE(stringStd < string);

    EXPECT_TRUE(string <= string);
    EXPECT_TRUE(string <= stringC);
    EXPECT_TRUE(stringC <= string);
    EXPECT_TRUE(string <= stringStd);
    EXPECT_TRUE(stringStd <= string);

    EXPECT_FALSE(string > string);
    EXPECT_FALSE(string > stringC);
    EXPECT_FALSE(stringC > string);
    EXPECT_FALSE(string > stringStd);
    EXPECT_FALSE(stringStd > string);

    EXPECT_TRUE(string >= string);
    EXPECT_TRUE(string >= stringC);
    EXPECT_TRUE(stringC >= string);
    EXPECT_TRUE(string >= stringStd);
    EXPECT_TRUE(stringStd >= string);
}

TEST(BoundedStringTest, TestCaseInsensitiveCompare)
{
    infra::BoundedConstString string("Abc");
    infra::BoundedConstString string2("Abc");

    EXPECT_FALSE(infra::CaseInsensitiveCompare(string, ""));

    EXPECT_TRUE(infra::CaseInsensitiveCompare(string, string2));
    EXPECT_TRUE(infra::CaseInsensitiveCompare(string, "abc"));
    EXPECT_TRUE(infra::CaseInsensitiveCompare(string, "ABC"));
}

TEST(BoundedStringTest, TestConcatenateWithStdString)
{
    EXPECT_EQ("ab", std::string("a") + infra::BoundedConstString("b"));
}

TEST(BoundedStringTest, TestReplaceByString)
{
    infra::BoundedString::WithStorage<10> string1("abcde");
    infra::BoundedString::WithStorage<10> string2("kl");
    string1.replace(1, 3, string2);
    EXPECT_EQ("akle", string1);
    string1.replace(string1.begin() + 1, string1.begin() + 2, std::string("xx"));
    EXPECT_EQ("axxle", string1);
    string1.replace(1, 1, "z");
    EXPECT_EQ("azxle", string1);
    string1.replace(1, 1, "mmmm", 1);
    EXPECT_EQ("amxle", string1);
    string1.replace(string1.begin() + 1, string1.begin() + 2, "u");
    EXPECT_EQ("auxle", string1);
    string1.replace(string1.begin() + 1, string1.begin() + 2, "vvv", 1);
    EXPECT_EQ("avxle", string1);
    string1.replace(1, 1, std::string("y"));
    EXPECT_EQ("ayxle", string1);
    string1.replace(string1.begin() + 1, string1.begin() + 2, std::string("aaa"), 1);
    EXPECT_EQ("aaxle", string1);
    string1.replace(1, 1, std::string("bbb"), 1);
    EXPECT_EQ("abxle", string1);
    string1.replace(string1.begin() + 1, string1.begin() + 2, infra::BoundedConstString("c"));
    EXPECT_EQ("acxle", string1);
    string1.replace(1, 1, infra::BoundedConstString("d"), 0, 1);
    EXPECT_EQ("adxle", string1);
}

TEST(BoundedStringTest, TestReplaceByChars)
{
    infra::BoundedString::WithStorage<5> string("abcd");
    string.replace(1, 2, 3, 'k');
    EXPECT_EQ("akkkd", string);
    string.replace(string.begin() + 1, string.begin() + 2, 1, 'x');
    EXPECT_EQ("axkkd", string);
}

TEST(BoundedStringTest, TestSubstr)
{
    infra::BoundedString::WithStorage<5> string("abcde");
    EXPECT_EQ("bcd", string.substr(1, 3));
}

TEST(BoundedStringTest, TestCopy)
{
    char buffer[3];
    infra::BoundedString::WithStorage<5> string("abcde");
    EXPECT_EQ(3, string.copy(buffer, 3, 1));
    EXPECT_EQ('b', buffer[0]);
}

TEST(BoundedStringTest, TestResize)
{
    infra::BoundedString::WithStorage<5> string("abc");
    string.resize(5, 'k');
    EXPECT_EQ("abckk", string);
    EXPECT_EQ(5, string.size());
}

TEST(BoundedStringTest, TestShrink)
{
    infra::BoundedConstString::WithStorage<5> string("abc");
    string.shrink(2);
    EXPECT_EQ("ab", string);
    EXPECT_EQ(2, string.size());
}

TEST(BoundedStringTest, TestSwap)
{
    infra::BoundedString::WithStorage<4> string1("abc");
    infra::BoundedString::WithStorage<5> string2("defg");
    swap(string1, string2);
    EXPECT_EQ("defg", string1);
    EXPECT_EQ("abc", string2);
}

TEST(BoundedStringTest, TestTrimLeft)
{
    EXPECT_EQ("abc ", infra::TrimLeft(infra::BoundedConstString("abc ")));
    EXPECT_EQ("abc ", infra::TrimLeft(infra::BoundedConstString("  abc ")));
    EXPECT_EQ("", infra::TrimLeft(infra::BoundedConstString("")));
    EXPECT_EQ("", infra::TrimLeft(infra::BoundedConstString(" ")));
}

TEST(BoundedStringTest, TestFind)
{
    infra::BoundedString::WithStorage<10> searchString = "def";
    infra::BoundedString::WithStorage<10> string("abcdefgh");
    EXPECT_EQ(3, string.find("def"));
    EXPECT_EQ(3, string.find(searchString));
}

TEST(BoundedStringTest, TestFindNotFound)
{
    infra::BoundedString::WithStorage<10> string("abcdefgh");
    EXPECT_EQ(infra::BoundedString::WithStorage<10>::npos, string.find("fed"));
}

TEST(BoundedStringTest, TestFindEmptyReturnsPos)
{
    infra::BoundedString::WithStorage<10> string("abcdefgh");
    EXPECT_EQ(4, string.find("", 4));
}

TEST(BoundedStringTest, TestRFind)
{
    infra::BoundedString::WithStorage<10> searchString("def");
    infra::BoundedString::WithStorage<10> string("abcdefgh");
    EXPECT_EQ(3, string.rfind("def"));
    EXPECT_EQ(3, string.rfind(searchString));
    EXPECT_EQ(3, string.rfind('d'));
}

TEST(BoundedStringTest, TestRFindNotFound)
{
    infra::BoundedString::WithStorage<10> string("abcdefgh");
    EXPECT_EQ(infra::BoundedString::WithStorage<10>::npos, string.rfind("fed"));
}

TEST(BoundedStringTest, TestRFindEmptyReturnsPos)
{
    infra::BoundedString::WithStorage<10> string("abcdefgh");
    EXPECT_EQ(4, string.rfind("", 4));
}

TEST(BoundedStringTest, TestFindFirstOf)
{
    char search[] = "dcb";
    infra::BoundedString::WithStorage<5> searchString = "dcb";
    infra::BoundedString::WithStorage<5> string("abcde");
    EXPECT_EQ(1, string.find_first_of(search));
    EXPECT_EQ(1, string.find_first_of(searchString));
    EXPECT_EQ(1, string.find_first_of('b'));
}

TEST(BoundedStringTest, TestFindFirstNotOf)
{
    char search[] = "dca";
    infra::BoundedString::WithStorage<5> searchString = "dca";
    infra::BoundedString::WithStorage<5> string("abcde");
    EXPECT_EQ(1, string.find_first_not_of(search));
    EXPECT_EQ(1, string.find_first_not_of(searchString));
    EXPECT_EQ(1, string.find_first_not_of('a'));
}

TEST(BoundedStringTest, TestFindLastOf)
{
    char search[] = "cdb";
    infra::BoundedString::WithStorage<5> searchString = "cdb";
    infra::BoundedString::WithStorage<5> string("abcde");
    EXPECT_EQ(3, string.find_last_of(search));
    EXPECT_EQ(3, string.find_last_of(searchString));
}

TEST(BoundedStringTest, TestFindLastNotOf)
{
    char search[] = "dce";
    infra::BoundedString::WithStorage<5> string("abcde");
    EXPECT_EQ(1, string.find_last_not_of(search));
    EXPECT_EQ(3, string.find_last_not_of('e'));
}

TEST(BoundedStringTest, TestStringAsByteRange)
{
    infra::BoundedString::WithStorage<5> string("abcde");

    EXPECT_EQ(infra::MemoryRange<uint8_t>(reinterpret_cast<uint8_t*>(string.begin()), reinterpret_cast<uint8_t*>(string.end())),
        StringAsByteRange(string));
    EXPECT_EQ(infra::MemoryRange<const uint8_t>(reinterpret_cast<const uint8_t*>(string.begin()), reinterpret_cast<const uint8_t*>(string.end())),
        StringAsByteRange(const_cast<const infra::BoundedString&>(static_cast<const infra::BoundedString&>(string))));
}

TEST(BoundedStringTest, TestByteRangeAsString)
{
    std::array<uint8_t, 4> data = { "str" };
    infra::MemoryRange<uint8_t> byteRange(data);
    infra::MemoryRange<const uint8_t> byteRange2(data);

    const auto boudedString = ByteRangeAsString(byteRange);
    const auto boundedConstString = ByteRangeAsString(infra::ConstByteRange(byteRange));

    EXPECT_EQ(boudedString[0], 's');
    EXPECT_EQ(boudedString[1], 't');
    EXPECT_EQ(boudedString[2], 'r');
    EXPECT_EQ(boudedString[3], '\0');

    EXPECT_EQ(boundedConstString[0], 's');
    EXPECT_EQ(boundedConstString[1], 't');
    EXPECT_EQ(boundedConstString[2], 'r');
    EXPECT_EQ(boundedConstString[3], '\0');
}

TEST(BoundedStringTest, TestStdStringAsByteRange)
{
    std::string string{ "abc" };
    const auto data = infra::StdStringAsByteRange(string);

    EXPECT_EQ(3, data.size());
    EXPECT_EQ('a', data[0]);
    EXPECT_EQ('b', data[1]);
    EXPECT_EQ('c', data[2]);
}

TEST(BoundedStringTest, TestPrintTo1)
{
    std::ostringstream stream;
    infra::BoundedString::WithStorage<5> string = "abc";
    infra::PrintTo(static_cast<infra::BoundedString&>(string), &stream);
    EXPECT_EQ(R"("abc")", stream.str());
}

TEST(BoundedStringTest, TestPrintTo2)
{
    std::ostringstream stream;
    infra::BoundedString::WithStorage<5> string = "abc";
    infra::PrintTo(string, &stream);
    EXPECT_EQ(R"("abc")", stream.str());
}
