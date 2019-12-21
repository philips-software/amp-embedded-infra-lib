#include "hal/generic/FileSystemGeneric.hpp"
#include "infra/stream/IoOutputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/util/Tokenizer.hpp"
#include "protobuf/echo_console/Console.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network_win/NameLookupWin.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include <fstream>
#include <iostream>

#undef GetObject

class ConfigParser
{
public:
    class ParseException
        : public std::runtime_error
    {
    public:
        ParseException(const std::string& message)
            : std::runtime_error(message)
        {}
    };

public:
    ConfigParser(hal::FileSystem& filesystem);

    void Open(const hal::filesystem::path& filename);

    std::string Hostname();

private:
    void CheckConfig();
    void CheckValidJson();
    void CheckMandatoryKeys();

private:
    hal::FileSystem& filesystem;
    std::string fileContents;
    infra::Optional<infra::JsonObject> json;
};

ConfigParser::ConfigParser(hal::FileSystem& filesystem)
    : filesystem(filesystem)
{}

void ConfigParser::Open(const hal::filesystem::path& filename)
{
    auto data = filesystem.ReadFile(filename);

    if (data.size() == 0)
        throw hal::EmptyFileException(filename);

    std::for_each(data.begin(), data.end(), [this](const std::string& line) { fileContents.append(line); });

    json.Emplace(fileContents);

    CheckConfig();
}

std::string ConfigParser::Hostname()
{
    return json->GetObject("echo_console").GetString("hostname").ToStdString();
}

void ConfigParser::CheckConfig()
{
    CheckValidJson();
    CheckMandatoryKeys();
}

void ConfigParser::CheckValidJson()
{
    for (auto it : *json)
    {}

    if (json->Error())
        throw ParseException("ConfigParser error: invalid JSON");
}

void ConfigParser::CheckMandatoryKeys()
{
    if (!json->HasKey("echo_console"))
        throw ParseException(std::string("ConfigParser error: required key echo_console missing"));
    if (!json->GetObject("echo_console").HasKey("hostname"))
        throw ParseException(std::string("ConfigParser error: required key echo_console/hostname missing"));
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
    virtual void Send(const std::string data) override;

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

void ConsoleClientConnection::Send(const std::string data)
{
    dataToBeSent += data;
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

using AllocatorConsole = infra::SharedObjectAllocator<ConsoleClientConnection, void(application::Console& console)>;

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

    try
    {
        application::EchoRoot root;

        for (int i = 1; i != argc; ++i)
        {
            std::ifstream stream(argv[i], std::ios::binary);
            if (!stream)
            {
                std::cerr << argv[0] << ": Could not read " << argv[i] << std::endl;
                return 1;
            }

            google::protobuf::FileDescriptorSet descriptorSet;
            if (!descriptorSet.ParseFromIstream(&stream)) {
                std::cerr << argv[0] << ": Could not parse contents from " << argv[i] << std::endl;
                return 1;
            }

            root.AddDescriptorSet(descriptorSet);
            std::cout << "Loaded " << argv[i] << std::endl;
        }

        hal::FileSystemGeneric filesystem;
        static ConfigParser config(filesystem);

        for (const char** var = env; *var != nullptr; ++var)
        {
            infra::Tokenizer tokenizer(*var, '=');
            if (tokenizer.Token(0) == "ECHO_CONSOLE_CONFIG_LOCATION")
                config.Open(tokenizer.Token(1).data());
        }

        application::Console console(root);
        services::NameLookupWin nameLookup;
        services::ConnectionFactoryWithNameResolverImpl::WithStorage<4> connectionFactory(console.ConnectionFactory(), nameLookup);
        infra::Optional<ConsoleClient> consoleClient;
        auto construct = [&]()
        {
            consoleClient.Emplace(connectionFactory, console, config.Hostname(), services::GlobalTracer());
        };
        infra::EventDispatcher::Instance().Schedule([&]() { construct(); });
        console.Run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
