#include "args.hxx"
#include "infra/stream/IoOutputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/util/Tokenizer.hpp"
#include "protobuf/echo_console/Console.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/util/MessageCommunicationCobs.hpp"
#include "services/util/MessageCommunicationWindowed.hpp"
#include <deque>
#include <fstream>
#include <iostream>

#ifdef ECHO_CONSOLE_WITH_UART
#include "hal/windows/UartWindows.hpp"
#endif

class ConsoleClientUart
    : public application::ConsoleObserver
    , private services::MessageCommunicationObserver
{
public:
    ConsoleClientUart(application::Console& console, hal::SerialCommunication& serial);
    ~ConsoleClientUart();

    // Implementation of ConsoleObserver
    virtual void Send(const std::string& message) override;

private:
    // Implementation of MessageCommunicationObserver
    virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
    virtual void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

private:
    void CheckDataToBeSent();

private:
    std::deque<std::string> messagesToBeSent;
    services::MessageCommunicationCobs::WithMaxMessageSize<2048> cobs;
    services::MessageCommunicationWindowed::WithReceiveBuffer<2048> windowed{ cobs };
    bool sending = false;
};

ConsoleClientUart::ConsoleClientUart(application::Console& console, hal::SerialCommunication& serial)
    : application::ConsoleObserver(console)
    , cobs(serial)
{
    services::MessageCommunicationObserver::Attach(windowed);
}

ConsoleClientUart::~ConsoleClientUart()
{
    services::MessageCommunicationObserver::Detach();
}

void ConsoleClientUart::Send(const std::string& message)
{
    messagesToBeSent.push_back(message);
    CheckDataToBeSent();
}

void ConsoleClientUart::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
{
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::StringAsByteRange(infra::BoundedConstString(messagesToBeSent.front()));
    writer = nullptr;

    messagesToBeSent.pop_front();
    sending = false;

    CheckDataToBeSent();
}

void ConsoleClientUart::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
{
    ConsoleObserver::Subject().DataReceived(*reader);
}

void ConsoleClientUart::CheckDataToBeSent()
{
    if (!sending && !messagesToBeSent.empty())
    {
        sending = true;
        windowed.RequestSendMessage(static_cast<uint16_t>(messagesToBeSent.front().size()));
    }
}

class ConsoleClientConnection
    : public services::ConnectionObserver
    , public application::ConsoleObserver
{
public:
    ConsoleClientConnection(application::Console& console);

    // Implementation of ConnectionObserver
    virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
    virtual void DataReceived() override;
    virtual void Attached() override;

    // Implementation of ConsoleObserver
    virtual void Send(const std::string& message) override;

private:
    void CheckDataToBeSent();

private:
    std::string dataToBeSent;
    infra::SharedPtr<infra::StreamWriter> writer;
};

ConsoleClientConnection::ConsoleClientConnection(application::Console& console)
    : application::ConsoleObserver(console)
{}

void ConsoleClientConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
{
    this->writer = writer;
    writer = nullptr;

    CheckDataToBeSent();
}

void ConsoleClientConnection::DataReceived()
{
    try
    {
        while (true)
        {
            auto stream = services::ConnectionObserver::Subject().ReceiveStream();
            ConsoleObserver::Subject().DataReceived(*stream);
            services::ConnectionObserver::Subject().AckReceived();
        }
    }
    catch (application::Console::IncompletePacket)
    {}
}

void ConsoleClientConnection::Attached()
{
    services::ConnectionObserver::Subject().RequestSendStream(services::ConnectionObserver::Subject().MaxSendStreamSize());
}

void ConsoleClientConnection::Send(const std::string& message)
{
    dataToBeSent += message;
    CheckDataToBeSent();
}

void ConsoleClientConnection::CheckDataToBeSent()
{
    if (writer != nullptr && !dataToBeSent.empty())
    {
        infra::TextOutputStream::WithErrorPolicy stream(*writer);
        std::size_t amount = std::min(stream.Available(), dataToBeSent.size());
        stream << dataToBeSent.substr(0, amount);
        dataToBeSent.erase(0, amount);

        writer = nullptr;
        services::ConnectionObserver::Subject().RequestSendStream(services::ConnectionObserver::Subject().MaxSendStreamSize());
    }
}

class ConsoleClient
    : public services::ClientConnectionObserverFactoryWithNameResolver
{
public:
    ConsoleClient(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console, std::string hostname, services::Tracer& tracer);
    ~ConsoleClient();

protected:
    virtual infra::BoundedConstString Hostname() const override;
    virtual uint16_t Port() const override;
    virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
    virtual void ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason) override;

private:
    services::ConnectionFactoryWithNameResolver& connectionFactory;
    infra::SharedOptional<ConsoleClientConnection> consoleClientConnection;
    infra::SharedPtr<void> connector;
    application::Console& console;
    std::string hostname;
    services::Tracer& tracer;
};

ConsoleClient::ConsoleClient(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console, std::string hostname, services::Tracer& tracer)
    : connectionFactory(connectionFactory)
    , console(console)
    , hostname(hostname)
    , tracer(tracer)
{
    tracer.Trace() << "Connecting to " << hostname;
    tracer.Trace();

    connectionFactory.Connect(*this);
}

ConsoleClient::~ConsoleClient()
{
    consoleClientConnection->services::ConnectionObserver::Subject().AbortAndDestroy();
}

infra::BoundedConstString ConsoleClient::Hostname() const
{
    return infra::BoundedConstString(hostname.data(), hostname.size());
}

uint16_t ConsoleClient::Port() const
{
    return 1234;
}

void ConsoleClient::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
{
    tracer.Trace() << "Connection established";
    tracer.Trace();
    createdObserver(consoleClientConnection.Emplace(console));
}

void ConsoleClient::ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason)
{
    tracer.Trace() << "Connection failed";
    exit(1);
}

int main(int argc, char* argv[], const char* env[])
{
    infra::IoOutputStream ioOutputStream;
    services::Tracer tracer(ioOutputStream);
    services::SetGlobalTracerInstance(tracer);

    std::string toolname = argv[0];
    args::ArgumentParser parser(toolname + " is a tool to communicate with an ECHO server.");
    args::Group arguments(parser, "Arguments:");
    args::Positional<std::string> target(arguments, "target", "COM port or hostname", args::Options::Required);
    args::Group flags(parser, "Optional flags:");
    args::HelpFlag help(flags, "help", "Display this help menu.", { 'h', "help" });
    args::PositionalList<std::string> paths(arguments, "paths", "compiled proto files");

    try
    {
        parser.Prog(toolname);
        parser.ParseCLI(argc, argv);

        application::EchoRoot root;

        for (const auto& path : paths)
        {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
            {
                std::cerr << argv[0] << ": Could not read " << path << std::endl;
                return 1;
            }

            google::protobuf::FileDescriptorSet descriptorSet;
            if (!descriptorSet.ParseFromIstream(&stream)) {
                std::cerr << argv[0] << ": Could not parse contents from " << path << std::endl;
                return 1;
            }

            root.AddDescriptorSet(descriptorSet);
            std::cout << "Loaded " << path << std::endl;
        }

        application::Console console(root);
        services::ConnectionFactoryWithNameResolverImpl::WithStorage<4> connectionFactory(console.ConnectionFactory(), console.NameResolver());
        infra::Optional<ConsoleClient> consoleClient;
#ifdef ECHO_CONSOLE_WITH_UART
        infra::Optional<hal::UartWindows> uart;
        infra::Optional<ConsoleClientUart> consoleClientUart;
#endif

        auto construct = [&]()
        {
            if (get(target).substr(0, 3) == "COM")
            {
#ifdef ECHO_CONSOLE_WITH_UART
                uart.Emplace(get(target));
                consoleClientUart.Emplace(console, *uart);
#else
                throw std::runtime_error("UART target not supported on this platform.");
#endif
            }
            else
                consoleClient.Emplace(connectionFactory, console, get(target), services::GlobalTracer());
        };
        infra::EventDispatcher::Instance().Schedule([&]() { construct(); });
        console.Run();
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
