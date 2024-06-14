#ifndef SERVICES_ECHO_CONSOLE_HPP
#define SERVICES_ECHO_CONSOLE_HPP

#include "hal/generic/TimerServiceGeneric.hpp"
#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "protobuf/protoc_echo_plugin/EchoObjects.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include <thread>

namespace application
{
    namespace ConsoleToken
    {
        class End
        {
        public:
            explicit End(std::size_t index);

            bool operator==(const End& other) const;
            bool operator!=(const End& other) const;

            std::size_t index;
        };

        class Error
        {
        public:
            explicit Error(std::size_t index);

            bool operator==(const Error& other) const;
            bool operator!=(const Error& other) const;

            std::size_t index;
        };

        class Comma
        {
        public:
            explicit Comma(std::size_t index);

            bool operator==(const Comma& other) const;
            bool operator!=(const Comma& other) const;

            std::size_t index;
        };

        class Dot
        {
        public:
            explicit Dot(std::size_t index);

            bool operator==(const Dot& other) const;
            bool operator!=(const Dot& other) const;

            std::size_t index;
        };

        class LeftBrace
        {
        public:
            explicit LeftBrace(std::size_t index);

            bool operator==(const LeftBrace& other) const;
            bool operator!=(const LeftBrace& other) const;

            std::size_t index;
        };

        class RightBrace
        {
        public:
            explicit RightBrace(std::size_t index);

            bool operator==(const RightBrace& other) const;
            bool operator!=(const RightBrace& other) const;

            std::size_t index;
        };

        class LeftBracket
        {
        public:
            explicit LeftBracket(std::size_t index);

            bool operator==(const LeftBracket& other) const;
            bool operator!=(const LeftBracket& other) const;

            std::size_t index;
        };

        class RightBracket
        {
        public:
            explicit RightBracket(std::size_t index);

            bool operator==(const RightBracket& other) const;
            bool operator!=(const RightBracket& other) const;

            std::size_t index;
        };

        class String
        {
        public:
            String(std::size_t index, const std::string& value);

            bool operator==(const String& other) const;
            bool operator!=(const String& other) const;

            std::size_t index;
            std::string value;
        };

        class Integer
        {
        public:
            Integer(std::size_t index, int32_t value);

            bool operator==(const Integer& other) const;
            bool operator!=(const Integer& other) const;

            std::size_t index;
            int64_t value;
        };

        class Boolean
        {
        public:
            Boolean(std::size_t index, bool value);

            bool operator==(const Boolean& other) const;
            bool operator!=(const Boolean& other) const;

            std::size_t index;
            bool value;
        };

        using Token = infra::Variant<End, Error, Comma, Dot, LeftBrace, RightBrace, LeftBracket, RightBracket, String, Integer, Boolean>;
    }

    class ConsoleTokenizer
    {
    public:
        explicit ConsoleTokenizer(const std::string& line);

        ConsoleToken::Token Token();

        bool operator==(const ConsoleTokenizer& other) const;
        bool operator!=(const ConsoleTokenizer& other) const;

    private:
        void SkipWhitespace();
        ConsoleToken::Token TryCreateStringToken();
        ConsoleToken::Token TryCreateIntegerToken();
        ConsoleToken::Token CreateIdentifierOrStringToken();

    private:
        std::string line;
        std::size_t parseIndex = 0;
    };

    class Console;

    class ConsoleObserver
        : public infra::SingleObserver<ConsoleObserver, Console>
    {
    public:
        using infra::SingleObserver<ConsoleObserver, Console>::SingleObserver;

        virtual void Send(const std::string& message) = 0;
    };

    class Console
        : public infra::Subject<ConsoleObserver>
    {
    public:
        explicit Console(EchoRoot& root);

        void Run();
        services::ConnectionFactory& ConnectionFactory();
        services::NameResolver& NameResolver();
        void DataReceived(infra::StreamReader& reader);

    private:
        struct MessageTokens
        {
            using MessageTokenValue = infra::Variant<std::string, int64_t, bool, MessageTokens, std::vector<MessageTokens>>;

            std::vector<std::pair<MessageTokenValue, std::size_t>> tokens;
        };

        class MethodInvocation
        {
        public:
            explicit MethodInvocation(const std::string& line);

            void EncodeParameters(std::shared_ptr<const EchoMessage> message, std::size_t lineSize, infra::ProtoFormatter& formatter);

            std::vector<std::string> method;
            MessageTokens messageTokens;

        private:
            void ProcessMethodTokens();
            void ProcessParameterTokens();
            std::pair<MessageTokens::MessageTokenValue, std::size_t> CreateMessageTokenValue();
            MessageTokens::MessageTokenValue ProcessMessage();
            std::vector<MessageTokens> ProcessArray();

            void EncodeMessage(const EchoMessage& message, const MessageTokens& messageTokens, std::size_t valueIndex, infra::ProtoFormatter& formatter);
            void EncodeField(const EchoField& field, const MessageTokens::MessageTokenValue& value, std::size_t valueIndex, infra::ProtoFormatter& formatter);

        private:
            ConsoleTokenizer tokenizer;
            ConsoleToken::Token currentToken;
        };

    private:
        void MethodReceived(const EchoService& service, const EchoMethod& method, infra::ProtoParser&& parser);
        void PrintMessage(const EchoMessage& message, infra::ProtoParser& parser);
        void PrintField(infra::Variant<uint32_t, uint64_t, infra::ProtoLengthDelimited>& fieldData, const EchoField& field, infra::ProtoParser& parser);
        void MethodNotFound(const EchoService& service, uint32_t methodId) const;
        void ServiceNotFound(uint32_t serviceId, uint32_t methodId) const;
        void RunEventDispatcher();
        void ListInterfaces();
        void ListFields(const EchoMessage& message);
        void Process(const std::string& line) const;
        std::pair<std::shared_ptr<const EchoService>, const EchoMethod&> SearchMethod(MethodInvocation& methodInvocation) const;

    private:
        EchoRoot& root;
        main_::NetworkAdapter network;
        hal::TimerServiceGeneric timerService{ infra::systemTimerServiceId };
        std::thread eventDispatcherThread;
        bool quit = false;
        std::mutex mutex;
        std::condition_variable condition;
        bool processDone = false;
        std::string receivedData;
    };

    namespace ConsoleExceptions
    {
        struct MethodNotFound
        {
            std::vector<std::string> method;
        };

        struct SyntaxError
        {
            std::size_t index;
        };

        struct TooManyParameters
        {
            std::size_t index;
        };

        struct MissingParameter
        {
            std::size_t index;
        };

        struct IncorrectType
        {
            std::size_t index;
        };
    }
}

#endif
