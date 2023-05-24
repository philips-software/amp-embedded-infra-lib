#include "services/echo_console/Console.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <string>

namespace application
{
    namespace
    {
        struct Quit
        {};
    }

    namespace ConsoleToken
    {
        End::End(std::size_t index)
            : index(index)
        {}

        bool End::operator==(const End&) const
        {
            return true;
        }

        bool End::operator!=(const End&) const
        {
            return false;
        }

        Error::Error(std::size_t index)
            : index(index)
        {}

        bool Error::operator==(const Error&) const
        {
            return true;
        }

        bool Error::operator!=(const Error&) const
        {
            return false;
        }

        Comma::Comma(std::size_t index)
            : index(index)
        {}

        bool Comma::operator==(const Comma&) const
        {
            return true;
        }

        bool Comma::operator!=(const Comma&) const
        {
            return false;
        }

        Dot::Dot(std::size_t index)
            : index(index)
        {}

        bool Dot::operator==(const Dot&) const
        {
            return true;
        }

        bool Dot::operator!=(const Dot&) const
        {
            return false;
        }

        LeftBrace::LeftBrace(std::size_t index)
            : index(index)
        {}

        bool LeftBrace::operator==(const LeftBrace& other) const
        {
            return index == other.index;
        }

        bool LeftBrace::operator!=(const LeftBrace& other) const
        {
            return index != other.index;
        }

        RightBrace::RightBrace(std::size_t index)
            : index(index)
        {}

        bool RightBrace::operator==(const RightBrace& other) const
        {
            return index == other.index;
        }

        bool RightBrace::operator!=(const RightBrace& other) const
        {
            return index != other.index;
        }

        LeftBracket::LeftBracket(std::size_t index)
            : index(index)
        {}

        bool LeftBracket::operator==(const LeftBracket& other) const
        {
            return index == other.index;
        }

        bool LeftBracket::operator!=(const LeftBracket& other) const
        {
            return index != other.index;
        }

        RightBracket::RightBracket(std::size_t index)
            : index(index)
        {}

        bool RightBracket::operator==(const RightBracket& other) const
        {
            return index == other.index;
        }

        bool RightBracket::operator!=(const RightBracket& other) const
        {
            return index != other.index;
        }

        String::String(std::size_t index, const std::string& value)
            : index(index)
            , value(value)
        {}

        bool String::operator==(const String& other) const
        {
            return value == other.value;
        }

        bool String::operator!=(const String& other) const
        {
            return value != other.value;
        }

        Integer::Integer(std::size_t index, int32_t value)
            : index(index)
            , value(value)
        {}

        bool Integer::operator==(const Integer& other) const
        {
            return value == other.value;
        }

        bool Integer::operator!=(const Integer& other) const
        {
            return value != other.value;
        }

        Boolean::Boolean(std::size_t index, bool value)
            : index(index)
            , value(value)
        {}

        bool Boolean::operator==(const Boolean& other) const
        {
            return value == other.value;
        }

        bool Boolean::operator!=(const Boolean& other) const
        {
            return value != other.value;
        }
    }

    namespace
    {
        struct IndexOfVisitor
            : public infra::StaticVisitor<std::size_t>
        {
            template<class T>
            std::size_t operator()(const T& value) const
            {
                return value.index;
            }
        };

        std::size_t IndexOf(const ConsoleToken::Token& token)
        {
            IndexOfVisitor visitor;
            return infra::ApplyVisitor(visitor, token);
        }
    }

    ConsoleTokenizer::ConsoleTokenizer(const std::string& line)
        : line(line)
    {}

    ConsoleToken::Token ConsoleTokenizer::Token()
    {
        SkipWhitespace();

        if (parseIndex == line.size())
            return ConsoleToken::End(parseIndex);
        else
        {
            if (line[parseIndex] == '"')
                return TryCreateStringToken();
            else if (line[parseIndex] == ',')
                return ConsoleToken::Comma(parseIndex++);
            else if (line[parseIndex] == '.')
                return ConsoleToken::Dot(parseIndex++);
            else if (line[parseIndex] == '{')
                return ConsoleToken::LeftBrace(parseIndex++);
            else if (line[parseIndex] == '}')
                return ConsoleToken::RightBrace(parseIndex++);
            else if (line[parseIndex] == '[')
                return ConsoleToken::LeftBracket(parseIndex++);
            else if (line[parseIndex] == ']')
                return ConsoleToken::RightBracket(parseIndex++);
            else if (std::isdigit(line[parseIndex]) != 0 || line[parseIndex] == '-')
                return TryCreateIntegerToken();
            else if (std::isalpha(line[parseIndex]) != 0)
                return CreateIdentifierOrStringToken();
            else
                return ConsoleToken::Error(parseIndex);
        }
    }

    bool ConsoleTokenizer::operator==(const ConsoleTokenizer& other) const
    {
        return parseIndex == other.parseIndex;
    }

    bool ConsoleTokenizer::operator!=(const ConsoleTokenizer& other) const
    {
        return !(*this == other);
    }

    void ConsoleTokenizer::SkipWhitespace()
    {
        while (parseIndex != line.size() && std::isspace(line[parseIndex]))
            ++parseIndex;
    }

    ConsoleToken::Token ConsoleTokenizer::TryCreateStringToken()
    {
        ++parseIndex;
        std::size_t tokenStart = parseIndex;

        bool escape = false;

        std::string stringToken;

        while (escape || line[parseIndex] != '"')
        {
            escape = !escape && line[parseIndex] == '\\';

            if (!escape)
                stringToken.append(1, line[parseIndex]);

            ++parseIndex;

            if (parseIndex == line.size())
                return ConsoleToken::Error(parseIndex);
        }

        ++parseIndex;

        return ConsoleToken::String(tokenStart, stringToken);
    }

    ConsoleToken::Token ConsoleTokenizer::TryCreateIntegerToken()
    {
        std::size_t tokenStart = parseIndex;
        bool sign = false;

        if (parseIndex != line.size() && line[parseIndex] == '-')
        {
            sign = true;
            ++parseIndex;
        }

        while (parseIndex != line.size() && std::isdigit(line[parseIndex]))
            ++parseIndex;

        std::string integer = line.substr(tokenStart, parseIndex - tokenStart);

        int32_t value = 0;
        for (std::size_t index = sign ? 1 : 0; index < integer.size(); ++index)
            value = value * 10 + integer[index] - '0';

        if (sign)
            value *= -1;

        return ConsoleToken::Integer(tokenStart, value);
    }

    ConsoleToken::Token ConsoleTokenizer::CreateIdentifierOrStringToken()
    {
        std::size_t tokenStart = parseIndex;

        while (parseIndex != line.size() && std::isalnum(line[parseIndex]))
            ++parseIndex;

        std::string identifier = line.substr(tokenStart, parseIndex - tokenStart);
        if (identifier == "true")
            return ConsoleToken::Boolean(tokenStart, true);
        else if (identifier == "false")
            return ConsoleToken::Boolean(tokenStart, false);
        else
            return ConsoleToken::String(tokenStart, identifier);
    }

    Console::Console(EchoRoot& root)
        : root(root)
        , eventDispatcherThread([this]()
              { RunEventDispatcher(); })
    {}

    void Console::Run()
    {
        while (!quit)
        {
            std::string line;
            std::getline(std::cin, line);

            if (line == "quit" || line == "exit")
                quit = true;
            else
            {
                std::unique_lock<std::mutex> lock(mutex);
                processDone = false;

                infra::EventDispatcher::Instance().Schedule([this, &line]()
                    {
                    if (line == "list")
                        ListInterfaces();
                    else
                        Process(line);

                    std::unique_lock<std::mutex> lock(mutex);
                    processDone = true;
                    condition.notify_all(); });

                while (!processDone)
                    condition.wait(lock);
            }
        }

        infra::EventDispatcher::Instance().Schedule([]()
            { throw Quit(); });
        eventDispatcherThread.join();
    }

    services::ConnectionFactory& Console::ConnectionFactory()
    {
        return network.ConnectionFactory();
    }

    services::NameResolver& Console::NameResolver()
    {
        return network.NameResolver();
    }

    void Console::DataReceived(infra::StreamReader& reader)
    {
        infra::DataInputStream::WithErrorPolicy data(reader, infra::softFail);
        infra::ProtoParser parser(data);
        auto serviceId = static_cast<uint32_t>(parser.GetVarInt());
        auto field = parser.GetField();

        if (data.Failed())
            throw IncompletePacket();

        for (const auto& service : root.services)
            if (service->serviceId == serviceId)
            {
                for (const auto& method : service->methods)
                    if (method.methodId == field.second)
                    {
                        MethodReceived(*service, method, field.first.Get<infra::ProtoLengthDelimited>().Parser());

                        data.Failed();
                        return;
                    }

                MethodNotFound(*service, field.second);
            }

        ServiceNotFound(serviceId, field.second);
    }

    void Console::MethodReceived(const EchoService& service, const EchoMethod& method, infra::ProtoParser&& parser)
    {
        std::cout << "Received " << service.name << "." << method.name << "(";

        if (method.parameter)
            PrintMessage(*method.parameter, parser);
        std::cout << ")" << std::endl;
    }

    void Console::PrintMessage(const EchoMessage& message, infra::ProtoParser& parser)
    {
        bool first = true;
        while (!parser.Empty())
        {
            if (!first)
                std::cout << ", ";
            first = false;

            auto field = parser.GetField();

            for (auto& echoField : message.fields)
            {
                if (echoField->number == field.second)
                    PrintField(field.first, *echoField, parser);
            }
        }
    }

    void Console::PrintField(infra::Variant<uint32_t, uint64_t, infra::ProtoLengthDelimited>& fieldData, const EchoField& field, infra::ProtoParser& parser)
    {
        class PrintFieldVisitor
            : public EchoFieldVisitor
        {
        public:
            PrintFieldVisitor(infra::Variant<uint32_t, uint64_t, infra::ProtoLengthDelimited>& fieldData, infra::ProtoParser& parser, Console& console)
                : fieldData(fieldData)
                , parser(parser)
                , console(console)
            {}

            void VisitInt64(const EchoFieldInt64& field) override
            {
                std::cout << static_cast<int32_t>(fieldData.Get<uint64_t>());
            }

            void VisitUint64(const EchoFieldUint64& field) override
            {
                std::cout << static_cast<int32_t>(fieldData.Get<uint64_t>());
            }

            void VisitInt32(const EchoFieldInt32& field) override
            {
                std::cout << static_cast<int32_t>(fieldData.Get<uint64_t>());
            }

            void VisitFixed64(const EchoFieldFixed64& field) override
            {
                std::cout << fieldData.Get<uint64_t>();
            }

            void VisitFixed32(const EchoFieldFixed32& field) override
            {
                std::cout << fieldData.Get<uint32_t>();
            }

            void VisitBool(const EchoFieldBool& field) override
            {
                std::cout << (fieldData.Get<uint64_t>() == 0 ? "false" : "true");
            }

            void VisitString(const EchoFieldString& field) override
            {
                std::cout << fieldData.Get<infra::ProtoLengthDelimited>().GetStdString();
            }

            void VisitUnboundedString(const EchoFieldUnboundedString& field) override
            {
                std::cout << fieldData.Get<infra::ProtoLengthDelimited>().GetStdString();
            }

            void VisitMessage(const EchoFieldMessage& field) override
            {
                std::cout << "{ ";
                infra::ProtoParser messageParser = fieldData.Get<infra::ProtoLengthDelimited>().Parser();
                console.PrintMessage(*field.message, messageParser);
                std::cout << " }";
            }

            void VisitBytes(const EchoFieldBytes& field) override
            {
                std::vector<uint8_t> bytes = fieldData.Get<infra::ProtoLengthDelimited>().GetUnboundedBytes();

                std::cout << "[";
                for (auto byte : bytes)
                    std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte);
                std::cout << "]";
            }

            void VisitUnboundedBytes(const EchoFieldUnboundedBytes& field) override
            {
                std::vector<uint8_t> bytes = fieldData.Get<infra::ProtoLengthDelimited>().GetUnboundedBytes();

                std::cout << "[";
                for (auto byte : bytes)
                    std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte);
                std::cout << "]";
            }

            void VisitUint32(const EchoFieldUint32& field) override
            {
                std::cout << fieldData.Get<uint64_t>();
            }

            void VisitEnum(const EchoFieldEnum& field) override
            {
                std::cout << fieldData.Get<uint64_t>();
            }

            void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                std::cout << static_cast<int64_t>(fieldData.Get<uint64_t>());
            }

            void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                std::cout << static_cast<int32_t>(fieldData.Get<uint32_t>());
            }

            void VisitRepeated(const EchoFieldRepeated& field) override
            {
                PrintFieldVisitor visitor(fieldData, parser, console);
                field.type->Accept(visitor);
            }

            void VisitUnboundedRepeated(const EchoFieldUnboundedRepeated& field) override
            {
                PrintFieldVisitor visitor(fieldData, parser, console);
                field.type->Accept(visitor);
            }

        private:
            infra::Variant<uint32_t, uint64_t, infra::ProtoLengthDelimited>& fieldData;
            infra::ProtoParser& parser;
            Console& console;
        };

        std::cout << field.name << " = ";
        PrintFieldVisitor visitor(fieldData, parser, *this);
        field.Accept(visitor);
    }

    void Console::MethodNotFound(const EchoService& service, uint32_t methodId) const
    {
        std::cout << "Received unknown method call for service " << service.name << " with id " << methodId << std::endl;
    }

    void Console::ServiceNotFound(uint32_t serviceId, uint32_t methodId) const
    {
        std::cout << "Received method call " << methodId << " for unknown service " << serviceId << std::endl;
    }

    void Console::RunEventDispatcher()
    {
        try
        {
            network.Run();
        }
        catch (Quit&)
        {}
    }

    void Console::ListInterfaces()
    {
        for (const auto& service : root.services)
        {
            services::GlobalTracer().Trace() << service->name;
            for (const auto& method : service->methods)
            {
                services::GlobalTracer().Trace() << "    " << method.name << "(";
                if (method.parameter != nullptr)
                    ListFields(*method.parameter);
                services::GlobalTracer().Continue() << ")";
            }
        }

        services::GlobalTracer().Trace();
    }

    void Console::ListFields(const EchoMessage& message)
    {
        struct ListFieldVisitor
            : public EchoFieldVisitor
        {
            explicit ListFieldVisitor(Console& console)
                : console(console)
            {}

            void VisitInt64(const EchoFieldInt64& field) override
            {
                services::GlobalTracer().Continue() << "int64";
            }

            void VisitUint64(const EchoFieldUint64& field) override
            {
                services::GlobalTracer().Continue() << "uint64";
            }

            void VisitInt32(const EchoFieldInt32& field) override
            {
                services::GlobalTracer().Continue() << "int32";
            }

            void VisitFixed64(const EchoFieldFixed64& field) override
            {
                services::GlobalTracer().Continue() << "fixed64";
            }

            void VisitFixed32(const EchoFieldFixed32& field) override
            {
                services::GlobalTracer().Continue() << "fixed32";
            }

            void VisitBool(const EchoFieldBool& field) override
            {
                services::GlobalTracer().Continue() << "bool";
            }

            void VisitString(const EchoFieldString& field) override
            {
                services::GlobalTracer().Continue() << "string[" << field.maxStringSize << "]";
            }

            void VisitUnboundedString(const EchoFieldUnboundedString& field) override
            {
                services::GlobalTracer().Continue() << "string";
            }

            void VisitMessage(const EchoFieldMessage& field) override
            {
                services::GlobalTracer().Continue() << "{ ";
                console.ListFields(*field.message);
                services::GlobalTracer().Continue() << " }";
            }

            void VisitBytes(const EchoFieldBytes& field) override
            {
                services::GlobalTracer().Continue() << "bytes[" << field.maxBytesSize << "]";
            }

            void VisitUnboundedBytes(const EchoFieldUnboundedBytes& field) override
            {
                services::GlobalTracer().Continue() << "bytes";
            }

            void VisitUint32(const EchoFieldUint32& field) override
            {
                services::GlobalTracer().Continue() << "uint32";
            }

            void VisitEnum(const EchoFieldEnum& field) override
            {
                services::GlobalTracer().Continue() << field.type->name;
            }

            void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                services::GlobalTracer().Continue() << "sfixed64";
            }

            void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                services::GlobalTracer().Continue() << "sfixed32";
            }

            void VisitRepeated(const EchoFieldRepeated& field) override
            {
                ListFieldVisitor visitor(console);
                field.type->Accept(visitor);
                services::GlobalTracer().Continue() << "[" << field.maxArraySize << "] ";
            }

            void VisitUnboundedRepeated(const EchoFieldUnboundedRepeated& field) override
            {
                ListFieldVisitor visitor(console);
                field.type->Accept(visitor);
            }

            Console& console;
        };

        ListFieldVisitor visitor(*this);

        for (auto& field : message.fields)
        {
            field->Accept(visitor);
            services::GlobalTracer().Continue() << " " << field->name;
            if (field != message.fields.back())
                services::GlobalTracer().Continue() << ", ";
        }
    }

    void Console::Process(const std::string& line) const
    {
        try
        {
            MethodInvocation methodInvocation(line);
            auto method = SearchMethod(methodInvocation);
            infra::ByteOutputStream::WithStorage<4096> stream;
            infra::ProtoFormatter formatter(stream);

            formatter.PutVarInt(method.first->serviceId);
            {
                auto subFormatter = formatter.LengthDelimitedFormatter(method.second.methodId);
                methodInvocation.EncodeParameters(method.second.parameter, line.size(), formatter);
            }

            auto range = infra::ReinterpretCastMemoryRange<char>(stream.Writer().Processed());
            GetObserver().Send(std::string(range.begin(), range.end()));
        }
        catch (ConsoleExceptions::SyntaxError& error)
        {
            services::GlobalTracer().Trace() << "Syntax error at index " << error.index << " (contents after that position is " << line.substr(error.index) << ")\n";
        }
        catch (ConsoleExceptions::TooManyParameters& error)
        {
            services::GlobalTracer().Trace() << "Too many parameters at index " << error.index << " (contents after that position is " << line.substr(error.index) << ")\n";
        }
        catch (ConsoleExceptions::MissingParameter& error)
        {
            services::GlobalTracer().Trace() << "Missing parameter at index " << error.index << " (contents after that position is " << line.substr(error.index) << ")\n";
        }
        catch (ConsoleExceptions::IncorrectType& error)
        {
            services::GlobalTracer().Trace() << "Incorrect type at index " << error.index << " (contents after that position is " << line.substr(error.index) << ")\n";
        }
        catch (ConsoleExceptions::MethodNotFound& error)
        {
            services::GlobalTracer().Trace() << "Method ";

            for (const auto& part : error.method)
            {
                if (&part != &error.method.front())
                    services::GlobalTracer().Continue() << ".";
                services::GlobalTracer().Continue() << part;
            }

            services::GlobalTracer().Continue() << " was not found\n";
        }
    }

    std::pair<std::shared_ptr<const EchoService>, const EchoMethod&> Console::SearchMethod(MethodInvocation& methodInvocation) const
    {
        for (auto service : root.services)
            for (auto& method : service->methods)
                if (method.name == methodInvocation.method.back())
                    return std::pair<std::shared_ptr<const EchoService>, const EchoMethod&>(service, method);

        throw ConsoleExceptions::MethodNotFound{ methodInvocation.method };
    }

    Console::MethodInvocation::MethodInvocation(const std::string& line)
        : tokenizer(line)
        , currentToken(tokenizer.Token())
    {
        ProcessMethodTokens();
        ProcessParameterTokens();
    }

    void Console::MethodInvocation::EncodeParameters(std::shared_ptr<const EchoMessage> message, std::size_t lineSize, infra::ProtoFormatter& formatter)
    {
        if (message)
            EncodeMessage(*message, messageTokens, lineSize, formatter);
        else if (!messageTokens.tokens.empty())
            throw ConsoleExceptions::TooManyParameters{ messageTokens.tokens.back().second };
    }

    void Console::MethodInvocation::ProcessMethodTokens()
    {
        while (true)
        {
            if (!currentToken.Is<ConsoleToken::String>())
                break;
            method.push_back(currentToken.Get<ConsoleToken::String>().value);

            currentToken = tokenizer.Token();

            if (!currentToken.Is<ConsoleToken::Dot>())
                break;

            currentToken = tokenizer.Token();
        }

        if (method.empty())
            throw ConsoleExceptions::SyntaxError{ 0 };
    }

    void Console::MethodInvocation::ProcessParameterTokens()
    {
        while (!currentToken.Is<ConsoleToken::End>())
        {
            messageTokens.tokens.push_back(CreateMessageTokenValue());
            currentToken = tokenizer.Token();
        }
    }

    std::pair<Console::MessageTokens::MessageTokenValue, std::size_t> Console::MethodInvocation::CreateMessageTokenValue()
    {
        struct TokenVisitor
            : public infra::StaticVisitor<Console::MessageTokens::MessageTokenValue>
        {
            explicit TokenVisitor(MethodInvocation& invocation)
                : invocation(invocation)
            {}

            MessageTokens::MessageTokenValue operator()(ConsoleToken::End)
            {
                std::abort();
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Error value)
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Comma value)
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Dot value)
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::LeftBrace)
            {
                invocation.currentToken = invocation.tokenizer.Token();
                return invocation.ProcessMessage();
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::RightBrace value)
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::LeftBracket)
            {
                invocation.currentToken = invocation.tokenizer.Token();
                return MessageTokens::MessageTokenValue(invocation.ProcessArray());
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::RightBracket value)
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::String value)
            {
                return MessageTokens::MessageTokenValue(value.value);
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Integer value)
            {
                return MessageTokens::MessageTokenValue(value.value);
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Boolean value)
            {
                return MessageTokens::MessageTokenValue(value.value);
            }

        private:
            MethodInvocation& invocation;
        };

        TokenVisitor visitor(*this);
        std::size_t index = IndexOf(currentToken);
        return std::make_pair(infra::ApplyVisitor(visitor, currentToken), index);
    }

    Console::MessageTokens::MessageTokenValue Console::MethodInvocation::ProcessMessage()
    {
        MessageTokens result;

        while (!currentToken.Is<ConsoleToken::End>() && !currentToken.Is<ConsoleToken::RightBrace>())
        {
            result.tokens.push_back(CreateMessageTokenValue());
            currentToken = tokenizer.Token();
        }

        if (!currentToken.Is<ConsoleToken::RightBrace>())
            throw ConsoleExceptions::SyntaxError{ IndexOf(currentToken) };
        currentToken = tokenizer.Token();

        return result;
    }

    std::vector<Console::MessageTokens> Console::MethodInvocation::ProcessArray()
    {
        std::vector<Console::MessageTokens> result;

        while (true)
        {
            Console::MessageTokens message;
            while (!currentToken.Is<ConsoleToken::End>() && !currentToken.Is<ConsoleToken::RightBracket>() && !currentToken.Is<ConsoleToken::Comma>())
            {
                message.tokens.push_back(CreateMessageTokenValue());
                currentToken = tokenizer.Token();
            }

            result.push_back(message);
            if (!currentToken.Is<ConsoleToken::Comma>())
                break;

            currentToken = tokenizer.Token();
        }

        if (!currentToken.Is<ConsoleToken::RightBracket>())
            throw ConsoleExceptions::SyntaxError{ IndexOf(currentToken) };
        currentToken = tokenizer.Token();

        return result;
    }

    void Console::MethodInvocation::EncodeMessage(const EchoMessage& message, const MessageTokens& messageTokens, std::size_t valueIndex, infra::ProtoFormatter& formatter)
    {
        auto tokens(messageTokens.tokens);

        for (auto field : message.fields)
        {
            if (tokens.empty())
                throw ConsoleExceptions::MissingParameter{ valueIndex };
            EncodeField(*field, tokens.front().first, tokens.front().second, formatter);
            tokens.erase(tokens.begin());
        }
    }

    void Console::MethodInvocation::EncodeField(const EchoField& field, const MessageTokens::MessageTokenValue& value, std::size_t valueIndex, infra::ProtoFormatter& formatter)
    {
        struct EncodeFieldVisitor
            : public EchoFieldVisitor
        {
            EncodeFieldVisitor(const MessageTokens::MessageTokenValue& value, std::size_t valueIndex, infra::ProtoFormatter& formatter, MethodInvocation& methodInvocation)
                : value(value)
                , valueIndex(valueIndex)
                , formatter(formatter)
                , methodInvocation(methodInvocation)
            {}

            void VisitInt64(const EchoFieldInt64& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitUint64(const EchoFieldUint64& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitInt32(const EchoFieldInt32& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitFixed32(const EchoFieldFixed32& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutFixed32Field(static_cast<uint32_t>(value.Get<int64_t>()), field.number);
            }

            void VisitFixed64(const EchoFieldFixed64& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutFixed64Field(static_cast<uint64_t>(value.Get<int64_t>()), field.number);
            }

            void VisitBool(const EchoFieldBool& field) override
            {
                if (!value.Is<bool>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutVarIntField(value.Get<bool>(), field.number);
            }

            void VisitString(const EchoFieldString& field) override
            {
                if (!value.Is<std::string>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutStringField(infra::BoundedConstString(value.Get<std::string>().data(), value.Get<std::string>().size()), field.number);
            }

            void VisitUnboundedString(const EchoFieldUnboundedString& field) override
            {
                if (!value.Is<std::string>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutStringField(infra::BoundedConstString(value.Get<std::string>().data(), value.Get<std::string>().size()), field.number);
            }

            void VisitEnum(const EchoFieldEnum& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutFixed32Field(static_cast<uint32_t>(value.Get<int64_t>()), field.number);
            }

            void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutFixed64Field(static_cast<uint64_t>(value.Get<int64_t>()), field.number);
            }

            void VisitMessage(const EchoFieldMessage& field) override
            {
                if (!value.Is<MessageTokens>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                methodInvocation.EncodeMessage(*field.message, value.Get<MessageTokens>(), valueIndex, formatter);
            }

            void VisitBytes(const EchoFieldBytes& field) override
            {
                if (!value.Is<std::vector<MessageTokens>>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };
                std::vector<uint8_t> bytes;
                for (auto& messageTokens : value.Get<std::vector<MessageTokens>>())
                {
                    if (messageTokens.tokens.size() < 1)
                        throw ConsoleExceptions::MissingParameter{ valueIndex };
                    if (messageTokens.tokens.size() > 1)
                        throw ConsoleExceptions::TooManyParameters{ messageTokens.tokens[1].second };
                    if (!messageTokens.tokens.front().first.Is<int64_t>())
                        throw ConsoleExceptions::IncorrectType{ messageTokens.tokens[0].second };

                    bytes.push_back(static_cast<uint8_t>(messageTokens.tokens.front().first.Get<int64_t>()));
                }

                formatter.PutBytesField(infra::MakeRange(bytes), field.number);
            }

            void VisitUnboundedBytes(const EchoFieldUnboundedBytes& field) override
            {
                if (!value.Is<std::vector<MessageTokens>>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };
                std::vector<uint8_t> bytes;
                for (auto& messageTokens : value.Get<std::vector<MessageTokens>>())
                {
                    if (messageTokens.tokens.size() < 1)
                        throw ConsoleExceptions::MissingParameter{ valueIndex };
                    if (messageTokens.tokens.size() > 1)
                        throw ConsoleExceptions::TooManyParameters{ messageTokens.tokens[1].second };
                    if (!messageTokens.tokens.front().first.Is<int64_t>())
                        throw ConsoleExceptions::IncorrectType{ messageTokens.tokens[0].second };

                    bytes.push_back(static_cast<uint8_t>(messageTokens.tokens.front().first.Get<int64_t>()));
                }

                formatter.PutBytesField(infra::MakeRange(bytes), field.number);
            }

            void VisitUint32(const EchoFieldUint32& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitRepeated(const EchoFieldRepeated& field) override
            {
                if (!value.Is<std::vector<MessageTokens>>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                for (auto& messageTokens : value.Get<std::vector<MessageTokens>>())
                {
                    if (messageTokens.tokens.size() < 1)
                        throw ConsoleExceptions::MissingParameter{ valueIndex };
                    if (messageTokens.tokens.size() > 1)
                        throw ConsoleExceptions::TooManyParameters{ messageTokens.tokens[1].second };
                    if (!messageTokens.tokens.front().first.Is<int64_t>())
                        throw ConsoleExceptions::IncorrectType{ messageTokens.tokens.front().second };

                    EncodeFieldVisitor visitor(messageTokens, valueIndex, formatter, methodInvocation);
                    field.type->Accept(visitor);
                }
            }

            void VisitUnboundedRepeated(const EchoFieldUnboundedRepeated& field) override
            {
                if (!value.Is<std::vector<MessageTokens>>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex };

                for (auto& messageTokens : value.Get<std::vector<MessageTokens>>())
                {
                    if (messageTokens.tokens.size() < 1)
                        throw ConsoleExceptions::MissingParameter{ valueIndex };
                    if (messageTokens.tokens.size() > 1)
                        throw ConsoleExceptions::TooManyParameters{ messageTokens.tokens[1].second };
                    if (!messageTokens.tokens.front().first.Is<int64_t>())
                        throw ConsoleExceptions::IncorrectType{ messageTokens.tokens.front().second };

                    EncodeFieldVisitor visitor(messageTokens, valueIndex, formatter, methodInvocation);
                    field.type->Accept(visitor);
                }
            }

        private:
            const MessageTokens::MessageTokenValue& value;
            std::size_t valueIndex;
            infra::ProtoFormatter& formatter;
            MethodInvocation& methodInvocation;
        };

        EncodeFieldVisitor visitor(value, valueIndex, formatter, *this);
        field.Accept(visitor);
    }
}
