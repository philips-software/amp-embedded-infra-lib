#include "services/echo_console/Console.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/stream/StringInputStream.hpp"
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

        Underscore::Underscore(std::size_t index)
            : index(index)
        {}

        bool Underscore::operator==(const Underscore&) const
        {
            return true;
        }

        bool Underscore::operator!=(const Underscore&) const
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
            else if (line[parseIndex] == '_')
                return ConsoleToken::Underscore(parseIndex++);
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

        int32_t value = 0;
        if (parseIndex != line.size() && line.substr(parseIndex, 2) == "0x")
        {
            tokenStart += 2;
            parseIndex += 2;
            while (parseIndex != line.size() && std::isxdigit(line[parseIndex]))
                ++parseIndex;

            std::string integer = line.substr(tokenStart, parseIndex - tokenStart);

            for (std::size_t index = sign ? 1 : 0; index < integer.size(); ++index)
            {
                if (std::isdigit(integer[index]))
                    value = value * 16 + integer[index] - '0';
                else
                    value = value * 16 + std::tolower(integer[index]) - 'a' + 10;
            }
        }
        else
        {
            while (parseIndex != line.size() && std::isdigit(line[parseIndex]))
                ++parseIndex;

            std::string integer = line.substr(tokenStart, parseIndex - tokenStart);

            for (std::size_t index = sign ? 1 : 0; index < integer.size(); ++index)
                value = value * 10 + integer[index] - '0';
        }

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

    Console::Console(EchoRoot& root, bool stopOnNetworkClose)
        : root(root)
        , eventDispatcherThread([this, stopOnNetworkClose]()
              {
                  RunEventDispatcher(stopOnNetworkClose);
              })
    {}

    void Console::Run()
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            started = true;
        }

        while (!quit && !stoppedEventDispatcher)
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
                        condition.notify_all();
                    });

                while (!processDone && !stoppedEventDispatcher)
                    condition.wait(lock);
            }
        }

        infra::EventDispatcher::Instance().Schedule([]()
            {
                throw Quit();
            });
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
        while (!reader.Empty())
            receivedData += infra::ByteRangeAsStdString(reader.ExtractContiguousRange(std::numeric_limits<std::size_t>::max()));

        while (!receivedData.empty())
        {
            infra::StringInputStream data(receivedData, infra::softFail);
            infra::ProtoParser parser(data >> infra::data);

            auto serviceId = static_cast<uint32_t>(parser.GetVarInt());
            auto [value, methodId] = parser.GetField();

            if (data.Failed())
                break;

            for (const auto& service : root.services)
                if (service->serviceId == serviceId)
                {
                    for (const auto& method : service->methods)
                        if (method.methodId == methodId)
                        {
                            MethodReceived(*service, method, value.Get<infra::ProtoLengthDelimited>().Parser());

                            receivedData.erase(receivedData.begin(), receivedData.begin() + data.Reader().ConstructSaveMarker());
                            data.Failed();
                            return;
                        }

                    MethodNotFound(*service, methodId);
                    receivedData.erase(receivedData.begin(), receivedData.begin() + data.Reader().ConstructSaveMarker());
                    return;
                }

            ServiceNotFound(serviceId, methodId);
            receivedData.erase(receivedData.begin(), receivedData.begin() + data.Reader().ConstructSaveMarker());
        }
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

            auto [value, methodId] = parser.GetField();

            for (auto& echoField : message.fields)
            {
                if (echoField->number == methodId)
                    PrintField(value, *echoField, parser);
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

            void VisitOptional(const EchoFieldOptional& field) override
            {
                PrintFieldVisitor visitor(fieldData, parser, console);
                field.type->Accept(visitor);
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

    void Console::RunEventDispatcher(bool stopOnNetworkClose)
    {
        try
        {
            network.ExecuteUntil([this, stopOnNetworkClose]()
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    return !network.NetworkActivity() && stopOnNetworkClose && started;
                });
        }
        catch (Quit&)
        {}

        std::unique_lock<std::mutex> lock(mutex);
        stoppedEventDispatcher = true;
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

            void VisitOptional(const EchoFieldOptional& field) override
            {
                ListFieldVisitor visitor(console);
                field.type->Accept(visitor);
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

            auto [service, method] = SearchMethod(methodInvocation);
            infra::StdVectorOutputStream::WithStorage stream;
            infra::ProtoFormatter formatter(stream);

            formatter.PutVarInt(service->serviceId);
            {
                auto subFormatter = formatter.LengthDelimitedFormatter(method.methodId);
                methodInvocation.EncodeParameters(method.parameter, line.size(), formatter);
            }

            GetObserver().Send(infra::ByteRangeAsStdString(infra::MakeRange(stream.Storage())));
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
            services::GlobalTracer().Trace() << "Missing parameter at index " << error.index << " of type " << error.missingType << " (contents after that position is " << line.substr(error.index) << ")\n";
        }
        catch (ConsoleExceptions::IncorrectType& error)
        {
            services::GlobalTracer().Trace() << "Incorrect type at index " << error.index << " expected type " << error.correctType << " (contents after that position is " << line.substr(error.index) << ")\n";
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
        {
            if (methodInvocation.method.size() == 1 || methodInvocation.method.front() == service->name)
                for (auto& method : service->methods)
                    if (method.name == methodInvocation.method.back())
                        return std::pair<std::shared_ptr<const EchoService>, const EchoMethod&>(service, method);
        }

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
            auto stringToken = currentToken.Get<ConsoleToken::String>();
            method.push_back(stringToken.value);

            if (method.size() > 2)
                throw ConsoleExceptions::SyntaxError{ stringToken.index };

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

            MessageTokens::MessageTokenValue operator()(ConsoleToken::End) const
            {
                std::abort();
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Error value) const
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Comma value) const
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Dot value) const
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Underscore value) const
            {
                return Empty{};
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::LeftBrace) const
            {
                invocation.currentToken = invocation.tokenizer.Token();
                return invocation.ProcessMessage();
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::RightBrace value) const
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::LeftBracket) const
            {
                invocation.currentToken = invocation.tokenizer.Token();
                return MessageTokens::MessageTokenValue(invocation.ProcessArray());
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::RightBracket value) const
            {
                throw ConsoleExceptions::SyntaxError{ value.index };
            }

            MessageTokens::MessageTokenValue operator()(const ConsoleToken::String& value) const
            {
                return MessageTokens::MessageTokenValue(value.value);
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Integer value) const
            {
                return MessageTokens::MessageTokenValue(value.value);
            }

            MessageTokens::MessageTokenValue operator()(ConsoleToken::Boolean value) const
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

        return result;
    }

    Console::MessageTokens Console::MethodInvocation::ProcessArray()
    {
        Console::MessageTokens result;

        while (true)
        {
            while (!currentToken.Is<ConsoleToken::End>() && !currentToken.Is<ConsoleToken::RightBracket>() && !currentToken.Is<ConsoleToken::Comma>())
            {
                result.tokens.push_back(CreateMessageTokenValue());
                currentToken = tokenizer.Token();
            }

            if (!currentToken.Is<ConsoleToken::Comma>())
                break;

            currentToken = tokenizer.Token();
        }

        if (!currentToken.Is<ConsoleToken::RightBracket>())
            throw ConsoleExceptions::SyntaxError{ IndexOf(currentToken) };

        return result;
    }

    void Console::MethodInvocation::EncodeMessage(const EchoMessage& message, const MessageTokens& messageTokens, std::size_t valueIndex, infra::ProtoFormatter& formatter)
    {
        auto tokens(messageTokens.tokens);

        for (auto field : message.fields)
        {
            if (tokens.empty())
                throw ConsoleExceptions::MissingParameter{ valueIndex, field->protoType };
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
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "integer" };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitUint64(const EchoFieldUint64& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "integer" };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitInt32(const EchoFieldInt32& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "integer" };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitFixed32(const EchoFieldFixed32& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "integer" };

                formatter.PutFixed32Field(static_cast<uint32_t>(value.Get<int64_t>()), field.number);
            }

            void VisitFixed64(const EchoFieldFixed64& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "integer" };

                formatter.PutFixed64Field(static_cast<uint64_t>(value.Get<int64_t>()), field.number);
            }

            void VisitBool(const EchoFieldBool& field) override
            {
                if (!value.Is<bool>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "bool" };

                formatter.PutVarIntField(value.Get<bool>(), field.number);
            }

            void VisitString(const EchoFieldString& field) override
            {
                if (!value.Is<std::string>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "string" };

                formatter.PutStringField(infra::BoundedConstString(value.Get<std::string>().data(), value.Get<std::string>().size()), field.number);
            }

            void VisitUnboundedString(const EchoFieldUnboundedString& field) override
            {
                if (!value.Is<std::string>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "string" };

                formatter.PutStringField(infra::BoundedConstString(value.Get<std::string>().data(), value.Get<std::string>().size()), field.number);
            }

            void VisitEnum(const EchoFieldEnum& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "integer" };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitSFixed32(const EchoFieldSFixed32& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "integer" };

                formatter.PutFixed32Field(static_cast<uint32_t>(value.Get<int64_t>()), field.number);
            }

            void VisitSFixed64(const EchoFieldSFixed64& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "integer" };

                formatter.PutFixed64Field(static_cast<uint64_t>(value.Get<int64_t>()), field.number);
            }

            void VisitMessage(const EchoFieldMessage& field) override
            {
                if (!value.Is<MessageTokens>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, field.protoType };

                infra::StdVectorOutputStream::WithStorage stream;
                infra::ProtoFormatter messageFormatter(stream);
                methodInvocation.EncodeMessage(*field.message, value.Get<MessageTokens>(), valueIndex, messageFormatter);
                formatter.PutLengthDelimitedField(infra::MakeRange(stream.Storage()), field.number);
            }

            void VisitBytes(const EchoFieldBytes& field) override
            {
                PutVector(field.number);
            }

            void VisitUnboundedBytes(const EchoFieldUnboundedBytes& field) override
            {
                PutVector(field.number);
            }

            void VisitUint32(const EchoFieldUint32& field) override
            {
                if (!value.Is<int64_t>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "integer" };

                formatter.PutVarIntField(value.Get<int64_t>(), field.number);
            }

            void VisitOptional(const EchoFieldOptional& field) override
            {
                if (!value.Is<Empty>())
                {
                    EncodeFieldVisitor visitor(value, valueIndex, formatter, methodInvocation);
                    field.type->Accept(visitor);
                }
            }

            void VisitRepeated(const EchoFieldRepeated& field) override
            {
                PutRepeated(field.protoType, field.type);
            }

            void VisitUnboundedRepeated(const EchoFieldUnboundedRepeated& field) override
            {
                PutRepeated(field.protoType, field.type);
            }

        private:
            void PutVector(int fieldNumber)
            {
                if (!value.Is<MessageTokens>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, "vector of integers" };
                std::vector<uint8_t> bytes;
                for (auto& messageToken : value.Get<MessageTokens>().tokens)
                {
                    if (!messageToken.first.Is<int64_t>())
                        throw ConsoleExceptions::IncorrectType{ messageToken.second, "integer" };

                    bytes.push_back(static_cast<uint8_t>(messageToken.first.Get<int64_t>()));
                }

                formatter.PutBytesField(infra::MakeRange(bytes), fieldNumber);
            }

            void PutRepeated(const std::string& fieldProtoType, std::shared_ptr<EchoField> fieldType)
            {
                if (!value.Is<std::vector<MessageTokens>>())
                    throw ConsoleExceptions::IncorrectType{ valueIndex, fieldProtoType };

                for (auto& messageTokens : value.Get<std::vector<MessageTokens>>())
                {
                    EncodeFieldVisitor visitor(messageTokens, valueIndex, formatter, methodInvocation);
                    fieldType->Accept(visitor);
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
