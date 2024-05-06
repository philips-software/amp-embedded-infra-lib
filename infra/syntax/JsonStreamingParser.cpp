#include "infra/syntax/JsonStreamingParser.hpp"
#include "infra/util/Function.hpp"
#include <cctype>

namespace infra
{
    void JsonObjectVisitor::VisitString(infra::BoundedConstString tag, infra::BoundedConstString value)
    {}

    void JsonObjectVisitor::VisitNumber(infra::BoundedConstString tag, int64_t value)
    {}

    void JsonObjectVisitor::VisitBoolean(infra::BoundedConstString tag, bool value)
    {}

    void JsonObjectVisitor::VisitNull(infra::BoundedConstString tag)
    {}

    JsonObjectVisitor* JsonObjectVisitor::VisitObject(infra::BoundedConstString tag, JsonSubObjectParser& parser)
    {
        return nullptr;
    }

    JsonArrayVisitor* JsonObjectVisitor::VisitArray(infra::BoundedConstString tag, JsonSubArrayParser& parser)
    {
        return nullptr;
    }

    void JsonObjectVisitor::Close()
    {}

    void JsonObjectVisitor::ParseError()
    {}

    void JsonObjectVisitor::SemanticError()
    {}

    void JsonObjectVisitor::StringOverflow()
    {}

    void JsonArrayVisitor::VisitString(infra::BoundedConstString value)
    {}

    void JsonArrayVisitor::VisitNumber(int64_t value)
    {}

    void JsonArrayVisitor::VisitBoolean(bool value)
    {}

    void JsonArrayVisitor::VisitNull()
    {}

    JsonObjectVisitor* JsonArrayVisitor::VisitObject(JsonSubObjectParser& parser)
    {
        return nullptr;
    }

    JsonArrayVisitor* JsonArrayVisitor::VisitArray(JsonSubArrayParser& parser)
    {
        return nullptr;
    }

    void JsonArrayVisitor::Close()
    {}

    void JsonArrayVisitor::ParseError()
    {}

    void JsonArrayVisitor::SemanticError()
    {}

    void JsonArrayVisitor::StringOverflow()
    {}

    JsonSubParser::JsonSubParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer,
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects)
        : tagBuffer(tagBuffer)
        , valueBuffer(valueBuffer)
        , subObjects(subObjects)
    {}

    JsonSubParser::~JsonSubParser()
    {
        if (destructedIndication != nullptr)
            *destructedIndication = true;
    }

    void JsonSubParser::FeedToken(infra::MemoryRange<const char>& data, bool saveValue)
    {
        while (!data.empty() && tokenState != TokenState::done)
        {
            auto c = data.front();

            if (tokenState == TokenState::numberOpen)
            {
                if (std::isdigit(c))
                {
                    tokenNumber = tokenNumber * 10 + c - '0';
                    data.pop_front();
                }
                else if (c == '.')
                {
                    tokenState = TokenState::numberFractionalOpen;
                    data.pop_front();
                }
                else
                    FoundToken(Token::number);
            }
            else if (tokenState == TokenState::numberFractionalOpen)
            {
                if (std::isdigit(c))
                    data.pop_front();
                else if (c == 'e' || c == 'E')
                {
                    tokenState = TokenState::numberExponentOpen;
                    data.pop_front();
                }
                else
                    FoundToken(Token::number);
            }
            else if (tokenState == TokenState::numberExponentOpen)
            {
                if (std::isdigit(c) || c == '+' || c == '-')
                    data.pop_front();
                else
                    FoundToken(Token::number);
            }
            else if (tokenState == TokenState::identifierOpen)
            {
                if (std::isalpha(c))
                {
                    AddToValueBuffer(c, saveValue, false);
                    data.pop_front();
                }
                else if (valueBuffer == "true")
                    FoundToken(Token::true_);
                else if (valueBuffer == "false")
                    FoundToken(Token::false_);
                else if (valueBuffer == "null")
                    FoundToken(Token::null);
                else
                    FoundToken(Token::error);
            }
            else
            {
                data.pop_front();

                switch (tokenState)
                {
                    case TokenState::open:
                        if (std::isspace(c))
                        {
                            while (!data.empty() && std::isspace(data.front()))
                                data.pop_front();
                        }
                        else
                            switch (c)
                            {
                                case ':':
                                    FoundToken(Token::colon);
                                    break;
                                case ',':
                                    FoundToken(Token::comma);
                                    break;
                                case '{':
                                    FoundToken(Token::leftBrace);
                                    break;
                                case '}':
                                    FoundToken(Token::rightBrace);
                                    break;
                                case '[':
                                    FoundToken(Token::leftBracket);
                                    break;
                                case ']':
                                    FoundToken(Token::rightBracket);
                                    break;
                                case '"':
                                    tokenState = TokenState::stringOpen;
                                    break;
                                case '-':
                                    tokenState = TokenState::numberOpen;
                                    tokenNumber = 0;
                                    tokenSign = -1;
                                    break;
                                default:
                                    if (std::isalpha(c))
                                    {
                                        tokenState = TokenState::identifierOpen;
                                        valueBuffer = c;
                                    }
                                    else if (std::isdigit(c))
                                    {
                                        tokenState = TokenState::numberOpen;
                                        tokenNumber = c - '0';
                                        tokenSign = 1;
                                    }
                                    else
                                        FoundToken(Token::error);
                            }
                        break;
                    case TokenState::stringOpen:
                    case TokenState::stringOverflowOpen:
                        while (true)
                        {
                            if (c == '\\')
                                tokenState = TokenState::stringOpenAndEscaped;
                            else if (c == '"')
                                FoundToken(tokenState == TokenState::stringOpen ? Token::string : Token::stringOverflow);
                            else
                                AddToValueBuffer(c, saveValue, true);

                            if ((tokenState != TokenState::stringOpen && tokenState != TokenState::stringOverflowOpen) || data.empty())
                                break;

                            c = data.front();
                            data.pop_front();
                        }
                        break;
                    case TokenState::stringOpenAndEscaped:
                        tokenState = TokenState::stringOpen;
                        ProcessEscapedData(c, saveValue);
                        break;
                    case TokenState::unicodeValueOpen:
                        if (c >= '0' && c <= '9')
                        {
                            ++unicodeIndex;
                            unicode = unicode * 16 + c - '0';
                        }
                        else if (c >= 'a' && c <= 'f')
                        {
                            ++unicodeIndex;
                            unicode = unicode * 16 + c - 'a' + 10;
                        }
                        else if (c >= 'A' && c <= 'F')
                        {
                            ++unicodeIndex;
                            unicode = unicode * 16 + c - 'A' + 10;
                        }
                        else
                        {
                            unicodeIndex = 0;
                            unicode = 0;
                            FoundToken(Token::error);
                        }

                        if (unicodeIndex == 4)
                        {
                            tokenState = TokenState::stringOpen;
                            if (valueBuffer.full())
                                FoundToken(Token::error);
                            else
                                AddToValueBuffer(static_cast<char>(unicode), saveValue, true);

                            unicodeIndex = 0;
                            unicode = 0;
                        }
                        break;
                    default:
                        std::abort();
                }
            }
        }
    }

    void JsonSubParser::ReportParseError()
    {
        bool destructed = false;
        destructedIndication = &destructed;

        for (auto subObject = subObjects.rbegin(); !destructed && subObject != subObjects.rend(); ++subObject)
            (*subObject)->Visitor().ParseError();

        if (!destructed)
        {
            subObjects.clear();
            destructedIndication = nullptr;
        }
    }

    void JsonSubParser::ReportSemanticError()
    {
        bool destructed = false;
        destructedIndication = &destructed;

        for (auto subObject = std::next(subObjects.rbegin()); !destructed && subObject != subObjects.rend(); ++subObject)
            (*subObject)->Visitor().SemanticError();

        if (!destructed)
        {
            subObjects.clear();
            destructedIndication = nullptr;
        }
    }

    infra::BoundedString JsonSubParser::CopyAndClear(infra::BoundedString& value) const
    {
        infra::BoundedString result(value);
        value.clear();
        return result;
    }

    void JsonSubParser::FoundToken(Token found)
    {
        token = found;
        tokenState = TokenState::done;
    }

    void JsonSubParser::ProcessEscapedData(char c, bool saveValue)
    {
        switch (c)
        {
            case '"':
                AddToValueBuffer('"', saveValue, true);
                break;
            case '\\':
                AddToValueBuffer('\\', saveValue, true);
                break;
            case 'b':
                AddToValueBuffer('\b', saveValue, true);
                break;
            case 'f':
                AddToValueBuffer('\f', saveValue, true);
                break;
            case 'n':
                AddToValueBuffer('\n', saveValue, true);
                break;
            case 'r':
                AddToValueBuffer('\r', saveValue, true);
                break;
            case 't':
                AddToValueBuffer('\t', saveValue, true);
                break;
            case 'u':
                tokenState = TokenState::unicodeValueOpen;
                break;
            default:
                AddToValueBuffer(c, saveValue, true);
        }
    }

    void JsonSubParser::AddToValueBuffer(char c, bool saveValue, bool inString)
    {
        if (valueBuffer.full())
        {
            if (inString)
                tokenState = TokenState::stringOverflowOpen;
            else
                FoundToken(Token::error);
        }
        else if (saveValue)
            valueBuffer += c;
    }

    JsonSubObjectParser::JsonSubObjectParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer,
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects, JsonObjectVisitor& visitor)
        : JsonSubParser(tagBuffer, valueBuffer, subObjects)
        , visitor(&visitor)
        , state(State::init)
    {}

    JsonSubObjectParser::JsonSubObjectParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer,
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects)
        : JsonSubParser(tagBuffer, valueBuffer, subObjects)
        , state(State::initialOpen)
    {
        this->tagBuffer.clear();
    }

    JsonSubObjectParser::~JsonSubObjectParser()
    {
        if (destructedIndication != nullptr)
            *destructedIndication = true;
    }

    void JsonSubObjectParser::Feed(infra::MemoryRange<const char>& data)
    {
        bool destructed = false;
        destructedIndication = &destructed;
        infra::ExecuteOnDestruction::WithExtraSize<3 * sizeof(void*)> execute([this, &destructed, &data]()
            {
                if (!destructed)
                    destructedIndication = nullptr;
            });

        while (!destructed && !data.empty() && state != State::parseError && state != State::semanticError)
        {
            FeedToken(data, state != State::skipNestedObject && state != State::skipNestedArray);
            if (tokenState != TokenState::done)
                break;

            if ((state == State::initialOpen || state == State::open) && (token == Token::string || token == Token::stringOverflow))
            {
                state = State::tagClosed;
                tagBuffer.assign(valueBuffer.substr(0, tagBuffer.max_size()));
            }
            else if (state == State::tagClosed && token == Token::colon)
                state = State::valueExpected;
            else if (state == State::valueExpected && token == Token::string)
            {
                state = State::closed;
                visitor->VisitString(CopyAndClear(tagBuffer), valueBuffer);
            }
            else if (state == State::valueExpected && token == Token::stringOverflow)
            {
                state = State::closed;
                tagBuffer.clear();
                visitor->StringOverflow();
            }
            else if (state == State::closed && token == Token::comma)
                state = State::open;
            else if (state == State::init)
            {
                if (token == Token::leftBrace)
                    state = State::initialOpen;
                else
                    state = State::parseError;
            }
            else if (state == State::valueExpected && token == Token::leftBrace)
            {
                tokenState = TokenState::open;
                state = State::closed;

                if (subObjects.full())
                    state = State::skipNestedObject;
                else
                {
                    subObjects.emplace_back(std::in_place_type_t<JsonSubObjectParser>(), tagBuffer, valueBuffer, subObjects);
                    auto nestedVisitor = visitor->VisitObject(tagBuffer, static_cast<JsonSubObjectParser&>(*subObjects.back()));
                    if (destructed)
                        return;

                    if (nestedVisitor != nullptr)
                        subObjects.back()->SetVisitor(*nestedVisitor);
                    else
                    {
                        subObjects.pop_back();
                        state = State::skipNestedObject;
                    }
                }

                tagBuffer.clear();
                return;
            }
            else if (state == State::valueExpected && token == Token::leftBracket)
            {
                tokenState = TokenState::open;
                state = State::closed;

                if (subObjects.full())
                    state = State::skipNestedArray;
                else
                {
                    subObjects.emplace_back(std::in_place_type_t<JsonSubArrayParser>(), tagBuffer, valueBuffer, subObjects);
                    auto nestedVisitor = visitor->VisitArray(tagBuffer, static_cast<JsonSubArrayParser&>(*subObjects.back()));
                    if (destructed)
                        return;

                    if (nestedVisitor != nullptr)
                        subObjects.back()->SetVisitor(*nestedVisitor);
                    else
                    {
                        subObjects.pop_back();
                        state = State::skipNestedArray;
                    }
                }

                tagBuffer.clear();
                return;
            }
            else if (state == State::valueExpected && token == Token::number)
            {
                state = State::closed;
                visitor->VisitNumber(CopyAndClear(tagBuffer), tokenSign * tokenNumber);
            }
            else if (state == State::valueExpected && token == Token::false_)
            {
                state = State::closed;
                visitor->VisitBoolean(CopyAndClear(tagBuffer), false);
            }
            else if (state == State::valueExpected && token == Token::true_)
            {
                state = State::closed;
                visitor->VisitBoolean(CopyAndClear(tagBuffer), true);
            }
            else if (state == State::valueExpected && token == Token::null)
            {
                state = State::closed;
                visitor->VisitNull(CopyAndClear(tagBuffer));
            }
            else if ((state == State::initialOpen || state == State::closed) && token == Token::rightBrace)
            {
                visitor->Close();
                if (!destructed)
                    subObjects.pop_back();
                return;
            }
            else if (state == State::skipNestedObject)
            {
                if (token == Token::rightBrace)
                {
                    if (skipSubObjects == 0)
                        state = State::closed;
                    else
                        --skipSubObjects;
                }
                else if (token == Token::leftBrace)
                    ++skipSubObjects;
            }
            else if (state == State::skipNestedArray)
            {
                if (token == Token::rightBracket)
                {
                    if (skipSubObjects == 0)
                        state = State::closed;
                    else
                        --skipSubObjects;
                }
                else if (token == Token::leftBracket)
                    ++skipSubObjects;
            }
            else
                state = State::parseError;

            valueBuffer.clear();
            tokenState = TokenState::open;
        }

        if (state == State::parseError)
        {
            data.clear();
            ReportParseError();
        }

        if (state == State::semanticError)
        {
            data.clear();
            ReportSemanticError();
        }
    }

    JsonVisitor& JsonSubObjectParser::Visitor()
    {
        return *visitor;
    }

    void JsonSubObjectParser::SetVisitor(JsonVisitor& visitor)
    {
        this->visitor = static_cast<JsonObjectVisitor*>(&visitor);
    }

    void JsonSubObjectParser::SemanticError()
    {
        state = State::semanticError;
    }

    JsonSubArrayParser::JsonSubArrayParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer,
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects, JsonArrayVisitor& visitor)
        : JsonSubParser(tagBuffer, valueBuffer, subObjects)
        , visitor(&visitor)
        , state(State::init)
    {}

    JsonSubArrayParser::JsonSubArrayParser(infra::BoundedString tagBuffer,
        infra::BoundedString valueBuffer, infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects)
        : JsonSubParser(tagBuffer, valueBuffer, subObjects)
    {}

    JsonSubArrayParser::~JsonSubArrayParser()
    {
        if (destructedIndication != nullptr)
            *destructedIndication = true;
    }

    void JsonSubArrayParser::Feed(infra::MemoryRange<const char>& data)
    {
        bool destructed = false;
        destructedIndication = &destructed;
        infra::ExecuteOnDestruction::WithExtraSize<3 * sizeof(void*)> execute([this, &destructed, &data]()
            {
                if (!destructed)
                    destructedIndication = nullptr;
            });

        while (!destructed && !data.empty() && state != State::parseError && state != State::semanticError)
        {
            FeedToken(data, state != State::skipNestedObject && state != State::skipNestedArray);
            if (tokenState != TokenState::done)
                break;

            if ((state == State::initialOpen || state == State::open) && token == Token::string)
            {
                state = State::closed;
                visitor->VisitString(valueBuffer);
            }
            else if ((state == State::initialOpen || state == State::open) && token == Token::stringOverflow)
            {
                state = State::closed;
                visitor->StringOverflow();
            }
            else if ((state == State::initialOpen || state == State::open) && token == Token::false_)
            {
                state = State::closed;
                visitor->VisitBoolean(false);
            }
            else if ((state == State::initialOpen || state == State::open) && token == Token::true_)
            {
                state = State::closed;
                visitor->VisitBoolean(true);
            }
            else if ((state == State::initialOpen || state == State::open) && token == Token::null)
            {
                state = State::closed;
                visitor->VisitNull();
            }
            else if ((state == State::initialOpen || state == State::open) && token == Token::number)
            {
                state = State::closed;
                tagBuffer.clear();
                visitor->VisitNumber(tokenSign * tokenNumber);
            }
            else if ((state == State::initialOpen || state == State::closed) && token == Token::rightBracket)
            {
                state = State::closed;
                visitor->Close();
                if (!destructed)
                    subObjects.pop_back();
                return;
            }
            else if ((state == State::initialOpen || state == State::open) && token == Token::leftBrace)
            {
                tokenState = TokenState::open;
                state = State::closed;

                if (subObjects.full())
                    state = State::skipNestedObject;
                else
                {
                    subObjects.emplace_back(std::in_place_type_t<JsonSubObjectParser>(), tagBuffer, valueBuffer, subObjects);
                    auto nestedVisitor = visitor->VisitObject(static_cast<JsonSubObjectParser&>(*subObjects.back()));
                    if (destructed)
                        return;

                    if (nestedVisitor != nullptr)
                        subObjects.back()->SetVisitor(*nestedVisitor);
                    else
                    {
                        subObjects.pop_back();
                        state = State::skipNestedObject;
                    }
                }

                return;
            }
            else if ((state == State::initialOpen || state == State::open) && token == Token::leftBracket)
            {
                tokenState = TokenState::open;
                state = State::closed;

                if (subObjects.full())
                    state = State::skipNestedArray;
                else
                {
                    subObjects.emplace_back(std::in_place_type_t<JsonSubArrayParser>(), tagBuffer, valueBuffer, subObjects);
                    auto nestedVisitor = visitor->VisitArray(static_cast<JsonSubArrayParser&>(*subObjects.back()));
                    if (destructed)
                        return;

                    if (nestedVisitor != nullptr)
                        subObjects.back()->SetVisitor(*nestedVisitor);
                    else
                    {
                        subObjects.pop_back();
                        state = State::skipNestedArray;
                    }
                }

                return;
            }
            else if (state == State::init)
            {
                if (token == Token::leftBracket)
                    state = State::initialOpen;
                else
                    state = State::parseError;
            }
            else if (state == State::skipNestedObject)
            {
                if (token == Token::rightBrace)
                {
                    if (skipSubObjects == 0)
                        state = State::closed;
                    else
                        --skipSubObjects;
                }
                else if (token == Token::leftBrace)
                    ++skipSubObjects;
            }
            else if (state == State::skipNestedArray)
            {
                if (token == Token::rightBracket)
                {
                    if (skipSubObjects == 0)
                        state = State::closed;
                    else
                        --skipSubObjects;
                }
                else if (token == Token::leftBracket)
                    ++skipSubObjects;
            }
            else if (state == State::closed && token == Token::comma)
                state = State::open;
            else
                state = State::parseError;

            valueBuffer.clear();
            tokenState = TokenState::open;
        }

        if (state == State::parseError)
        {
            data.clear();
            ReportParseError();
        }

        if (state == State::semanticError)
        {
            data.clear();
            ReportSemanticError();
        }
    }

    JsonVisitor& JsonSubArrayParser::Visitor()
    {
        return *visitor;
    }

    void JsonSubArrayParser::SetVisitor(JsonVisitor& visitor)
    {
        this->visitor = static_cast<JsonArrayVisitor*>(&visitor);
    }

    void JsonSubArrayParser::SemanticError()
    {
        state = State::semanticError;
    }

    JsonStreamingObjectParser::JsonStreamingObjectParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer,
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects, JsonObjectVisitor& visitor)
        : subObjects(subObjects)
    {
        this->subObjects.emplace_back(std::in_place_type_t<JsonSubObjectParser>(), tagBuffer, valueBuffer, subObjects, visitor);
    }

    void JsonStreamingObjectParser::Feed(infra::BoundedConstString data)
    {
        auto dataRange(infra::MakeRange(data));

        while (!dataRange.empty() && !subObjects.empty())
            subObjects.back()->Feed(dataRange);
    }

    JsonStreamingArrayParser::JsonStreamingArrayParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer,
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects, JsonArrayVisitor& visitor)
        : subObjects(subObjects)
    {
        this->subObjects.emplace_back(std::in_place_type_t<JsonSubArrayParser>(), tagBuffer, valueBuffer, subObjects, visitor);
    }

    void JsonStreamingArrayParser::Feed(infra::BoundedConstString data)
    {
        auto dataRange(infra::MakeRange(data));

        while (!dataRange.empty() && !subObjects.empty())
            subObjects.back()->Feed(dataRange);
    }

    JsonObjectVisitorDecorator::JsonObjectVisitorDecorator(JsonObjectVisitor& decorated)
        : decorated(decorated)
    {}

    void JsonObjectVisitorDecorator::VisitString(infra::BoundedConstString tag, infra::BoundedConstString value)
    {
        decorated.VisitString(tag, value);
    }

    void JsonObjectVisitorDecorator::VisitNumber(infra::BoundedConstString tag, int64_t value)
    {
        decorated.VisitNumber(tag, value);
    }

    void JsonObjectVisitorDecorator::VisitBoolean(infra::BoundedConstString tag, bool value)
    {
        decorated.VisitBoolean(tag, value);
    }

    void JsonObjectVisitorDecorator::VisitNull(infra::BoundedConstString tag)
    {
        decorated.VisitNull(tag);
    }

    JsonObjectVisitor* JsonObjectVisitorDecorator::VisitObject(infra::BoundedConstString tag, JsonSubObjectParser& parser)
    {
        return decorated.VisitObject(tag, parser);
    }

    JsonArrayVisitor* JsonObjectVisitorDecorator::VisitArray(infra::BoundedConstString tag, JsonSubArrayParser& parser)
    {
        return decorated.VisitArray(tag, parser);
    }

    void JsonObjectVisitorDecorator::Close()
    {
        decorated.Close();
    }

    void JsonObjectVisitorDecorator::ParseError()
    {
        decorated.ParseError();
    }

    void JsonObjectVisitorDecorator::SemanticError()
    {
        decorated.SemanticError();
    }

    void JsonObjectVisitorDecorator::StringOverflow()
    {
        decorated.StringOverflow();
    }
}
