#include "args.hxx"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#ifdef EMIL_HAL_WINDOWS
#include "hal/windows/UartWindows.hpp"
#else
#include "hal/unix/UartUnix.hpp"
#endif
#include "infra/stream/IoOutputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/util/Tokenizer.hpp"
#include "services/echo_console/Console.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network/WebSocketClientConnectionObserver.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"
#include <deque>
#include <fstream>
#include <iostream>

class ConsoleClientUart
    : public application::ConsoleObserver
    , private services::SesameObserver
{
public:
    ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial);

    // Implementation of ConsoleObserver
    void Send(const std::string& message) override;

private:
    // Implementation of SesameObserver
    void Initialized() override;
    void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
    void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

private:
    void CheckDataToBeSent();

private:
    std::deque<std::string> messagesToBeSent;
    services::SesameCobs::WithMaxMessageSize<2048> cobs;
    services::SesameWindowed windowed{ cobs };
    bool sending = false;
    services::SesameObserver::DelayedAttachDetach delayed{ *this, windowed };
};

ConsoleClientUart::ConsoleClientUart(application::Console& console, hal::BufferedSerialCommunication& serial)
    : application::ConsoleObserver(console)
    , cobs(serial)
{}

void ConsoleClientUart::Send(const std::string& message)
{
    auto size = windowed.MaxSendMessageSize();
    for (std::size_t index = 0; index < message.size(); index += size)
        messagesToBeSent.push_back(message.substr(index, size));
    CheckDataToBeSent();
}

void ConsoleClientUart::Initialized()
{}

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
    explicit ConsoleClientConnection(application::Console& console);

    // Implementation of ConnectionObserver
    void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
    void DataReceived() override;
    void Attached() override;

    // Implementation of ConsoleObserver
    void Send(const std::string& message) override;

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
    catch (application::Console::IncompletePacket&)
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

class ConsoleClientTcp
    : private services::ClientConnectionObserverFactoryWithNameResolver
{
public:
    ConsoleClientTcp(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console, const std::string& hostname, services::Tracer& tracer);
    ~ConsoleClientTcp();

private:
    infra::BoundedConstString Hostname() const override;
    uint16_t Port() const override;
    void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
    void ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason) override;

private:
    infra::SharedOptional<ConsoleClientConnection> consoleClientConnection;
    infra::SharedPtr<void> connector;
    application::Console& console;
    std::string hostname;
    services::Tracer& tracer;
};

ConsoleClientTcp::ConsoleClientTcp(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console, const std::string& hostname, services::Tracer& tracer)
    : console(console)
    , hostname(hostname)
    , tracer(tracer)
{
    tracer.Trace() << "Connecting to " << hostname;
    tracer.Trace();

    connectionFactory.Connect(*this);
}

ConsoleClientTcp::~ConsoleClientTcp()
{
    consoleClientConnection->services::ConnectionObserver::Subject().AbortAndDestroy();
}

infra::BoundedConstString ConsoleClientTcp::Hostname() const
{
    return infra::BoundedConstString(hostname.data(), hostname.size());
}

uint16_t ConsoleClientTcp::Port() const
{
    return 1234;
}

void ConsoleClientTcp::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
{
    tracer.Trace() << "Connection established";
    tracer.Trace();
    createdObserver(consoleClientConnection.Emplace(console));
}

void ConsoleClientTcp::ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason)
{
    tracer.Trace() << "Connection failed";
    exit(1);
}

class ConsoleClientWebSocket
    : private services::WebSocketClientObserverFactory
{
public:
    ConsoleClientWebSocket(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console,
        infra::BoundedString url, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer);

private:
    infra::BoundedString Url() const override;
    uint16_t Port() const override;
    void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClientObserver) override;
    void ConnectionFailed(ConnectFailReason reason) override;

private:
    infra::BoundedString url;
    services::HttpClientConnectorWithNameResolverImpl<> clientConnector;
    infra::Creator<services::Stoppable, services::HttpClientWebSocketInitiation, void(services::WebSocketClientObserverFactory& clientObserverFactory, services::HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)> httpClientInitiationCreator;
    services::WebSocketClientFactorySingleConnection webSocketFactory;
    services::Tracer& tracer;

    infra::SharedOptional<ConsoleClientConnection> consoleClientConnection;
    application::Console& console;
};

ConsoleClientWebSocket::ConsoleClientWebSocket(services::ConnectionFactoryWithNameResolver& connectionFactory, application::Console& console,
    infra::BoundedString url, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
    : url(url)
    , clientConnector(connectionFactory)
    , httpClientInitiationCreator(
          [this](infra::Optional<services::HttpClientWebSocketInitiation>& value, services::WebSocketClientObserverFactory& clientObserverFactory,
              services::HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)
          {
              value.Emplace(clientObserverFactory, clientConnector, result, randomDataGenerator);
          })
    , webSocketFactory(randomDataGenerator, { httpClientInitiationCreator })
    , tracer(tracer)
    , console(console)
{
    webSocketFactory.Connect(*this);
}

infra::BoundedString ConsoleClientWebSocket::Url() const
{
    return url;
}

uint16_t ConsoleClientWebSocket::Port() const
{
    return 80;
}

void ConsoleClientWebSocket::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClientObserver)
{
    tracer.Trace() << "Connection established";
    tracer.Trace();
    createdClientObserver(consoleClientConnection.Emplace(console));
}

void ConsoleClientWebSocket::ConnectionFailed(ConnectFailReason reason)
{
    tracer.Trace() << "Connection failed";
    exit(1);
}

int main(int argc, char* argv[], const char* env[])
{
    infra::IoOutputStream ioOutputStream;
    services::Tracer tracer(ioOutputStream);
    services::SetGlobalTracerInstance(tracer);
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;

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
            if (!descriptorSet.ParseFromIstream(&stream))
            {
                std::cerr << argv[0] << ": Could not parse contents from " << path << std::endl;
                return 1;
            }

            root.AddDescriptorSet(descriptorSet);
            std::cout << "Loaded " << path << std::endl;
        }

        application::Console console(root);
        services::ConnectionFactoryWithNameResolverImpl::WithStorage<4> connectionFactory(console.ConnectionFactory(), console.NameResolver());
        infra::Optional<ConsoleClientTcp> consoleClientTcp;
        infra::Optional<ConsoleClientWebSocket> consoleClientWebSocket;
#ifdef EMIL_HAL_WINDOWS
        infra::Optional<hal::UartWindows> uart;
#else
        infra::Optional<hal::UartUnix> uart;
#endif
        infra::Optional<hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<2048>> bufferedUart;
        infra::Optional<ConsoleClientUart> consoleClientUart;

        auto construct = [&]()
        {
            if (get(target).substr(0, 3) == "COM" || get(target).substr(0, 4) == "/dev")
            {
                uart.Emplace(get(target));
                bufferedUart.Emplace(*uart);
                consoleClientUart.Emplace(console, *bufferedUart);
            }
            else if (services::SchemeFromUrl(get(target)) == "ws")
                consoleClientWebSocket.Emplace(connectionFactory, console, get(target), randomDataGenerator, tracer);
            else
                consoleClientTcp.Emplace(connectionFactory, console, get(target), tracer);
        };

        infra::EventDispatcher::Instance().Schedule([&construct]()
            {
                construct();
            });
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
