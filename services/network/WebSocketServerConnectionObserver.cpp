#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "mbedtls/sha1.h"
#include "services/network/HttpServer.hpp"
#include "services/network/WebSocketServerConnectionObserver.hpp"
#include <cassert>

namespace services
{
    WebSocketServerConnectionObserver::WebSocketServerConnectionObserver(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedDeque<uint8_t>& receiveBuffer, services::Connection& connection, infra::BoundedConstString handshakeKey)
        : receivingState(infra::InPlaceType<ReceivingStateInitial>(), *this, handshakeKey)
        , sendingState(infra::InPlaceType<SendingStateIdle>(), *this)
        , receiveBuffer(receiveBuffer)
        , sendBuffer(sendBuffer)
        , streamReader([this]() { ReceiveStreamAllocatable(); })
        , streamWriter([this]() { SendStreamAllocatable(); })
    {
        connection.GetObserver().Detach();
        services::ConnectionObserver::Attach(connection);

        sendingState->CheckForSomethingToDo();
    }

    void WebSocketServerConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        sendingState->SendStreamAvailable(std::move(writer));
    }

    void WebSocketServerConnectionObserver::DataReceived()
    {
        auto startSize = receiveBuffer.size();

        receivingState->DataReceived();
        sendingState->CheckForSomethingToDo();

        if (startSize != receiveBuffer.size())
            services::Connection::GetObserver().DataReceived();
    }

    void WebSocketServerConnectionObserver::ClosingConnection()
    {
        ResetOwnership();
    }

    void WebSocketServerConnectionObserver::RequestSendStream(std::size_t sendSize)
    {
        assert(sendSize != 0 && sendSize <= MaxSendStreamSize());
        assert(requestedSendSize == 0);
        requestedSendSize = sendSize;

        TryAllocateSendStream();
    }

    std::size_t WebSocketServerConnectionObserver::MaxSendStreamSize() const
    {
        return sendBuffer.max_size();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> WebSocketServerConnectionObserver::ReceiveStream()
    {
        assert(!streamReader);
        return streamReader.Emplace(infra::inPlace, receiveBuffer, receiveBuffer.size());
    }

    void WebSocketServerConnectionObserver::AckReceived()
    {
        assert(streamReader);
        receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + streamReader->Storage().Processed());
        streamReader->ResetLength(receiveBuffer.size());
    }

    void WebSocketServerConnectionObserver::CloseAndDestroy()
    {
        Close();
    }

    void WebSocketServerConnectionObserver::AbortAndDestroy()
    {
        Abort();
    }

    void WebSocketServerConnectionObserver::ReceiveStreamAllocatable()
    {
        receivingState->DataReceived();
    }

    void WebSocketServerConnectionObserver::SendStreamAllocatable()
    {
        sendBufferReadyForSending = !sendBuffer.empty();
        sendingState->CheckForSomethingToDo();
    }

    void WebSocketServerConnectionObserver::SetReceivingStateReceiveHeader()
    {
        receivingState.Emplace<ReceivingStateReceiveHeader>(*this);
        receivingState->DataReceived();
    }

    void WebSocketServerConnectionObserver::SetStateReceiveData(services::WebSocketFrameHeader header)
    {
        receivingState.Emplace<ReceivingStateReceiveData>(*this, header);
        receivingState->DataReceived();
    }

    void WebSocketServerConnectionObserver::SetReceivingStateClose()
    {
        receivingState.Emplace<ReceivingStateClose>(*this);
    }

    void WebSocketServerConnectionObserver::SetReceivingStatePong()
    {
        receivingState.Emplace<ReceivingStatePong>(*this);
    }

    void WebSocketServerConnectionObserver::SetStateSendingIdle()
    {
        sendingState.Emplace<SendingStateIdle>(*this);
        sendingState->CheckForSomethingToDo();
    }

    void WebSocketServerConnectionObserver::SendFrame(services::WebSocketOpCode operationCode, infra::ConstByteRange data, infra::StreamWriter& writer) const
    {
        infra::DataOutputStream::WithErrorPolicy stream(writer);
        stream << operationCode;

        uint32_t dataLength = data.size();
        if (dataLength <= 125)
            stream << static_cast<uint8_t>(dataLength);
        else
            stream << static_cast<uint8_t>(126) << static_cast<uint8_t>(dataLength >> 8) << static_cast<uint8_t>(dataLength);

        stream << data;
    }

    void WebSocketServerConnectionObserver::TryAllocateSendStream()
    {
        if (streamWriter.Allocatable() && sendBuffer.empty() && requestedSendSize != 0)
        {
            services::Connection::GetObserver().SendStreamAvailable(streamWriter.Emplace(infra::inPlace, sendBuffer, requestedSendSize));
            requestedSendSize = 0;
        }
    }

    void WebSocketServerConnectionObserver::ReceivingState::DataReceived()
    {}

    WebSocketServerConnectionObserver::ReceivingStateThatSendsData::ReceivingStateThatSendsData(WebSocketServerConnectionObserver& connection)
    {
        connection.receivingStateThatWantsToSendData = this;
    }

    WebSocketServerConnectionObserver::ReceivingStateInitial::ReceivingStateInitial(WebSocketServerConnectionObserver& connection, infra::BoundedConstString handshakeKey)
        : ReceivingStateThatSendsData(connection)
        , connection(connection)
        , handshakeKey(handshakeKey)
    {}

    void WebSocketServerConnectionObserver::ReceivingStateInitial::Send(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::BoundedString::WithStorage<MaxWebSocketKeySize> webSocketKey = handshakeKey;
        static const infra::BoundedConstString webSocketGuid("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        webSocketKey.append(webSocketGuid);

        std::array<uint8_t, 20> sha1Digest;
        mbedtls_sha1(reinterpret_cast<const uint8_t*>(webSocketKey.begin()), webSocketKey.size(), sha1Digest.data());    //TICS !INT#030

        infra::TextOutputStream::WithErrorPolicy stream(*writer);
        services::HttpResponseHeaderBuilder responseBuilder(stream, "101 Switching Protocols");
        responseBuilder.AddHeader("Upgrade", "websocket");
        responseBuilder.AddHeader("Connection", "Upgrade");
        responseBuilder.AddHeader("Sec-WebSocket-Accept");
        stream << infra::AsBase64(sha1Digest);
        responseBuilder.StartBody();

        connection.SetReceivingStateReceiveHeader();
    }

    WebSocketServerConnectionObserver::ReceivingStateReceiveHeader::ReceivingStateReceiveHeader(WebSocketServerConnectionObserver& connection)
        : connection(connection)
    {}

    void WebSocketServerConnectionObserver::ReceivingStateReceiveHeader::DataReceived()
    {
        infra::SharedPtr<infra::StreamReader> reader = connection.services::ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::softFail);
        services::WebSocketFrameHeader header(stream);
        if (stream.Failed())
            return;

        connection.services::ConnectionObserver::Subject().AckReceived();
        reader = nullptr;

        if (!header.IsValid())
            connection.SetReceivingStateClose();
        else
            connection.SetStateReceiveData(header);
    }

    WebSocketServerConnectionObserver::ReceivingStateReceiveData::ReceivingStateReceiveData(WebSocketServerConnectionObserver& connection, const services::WebSocketFrameHeader& header)
        : connection(connection)
        , header(header)
        , sizeToReceive(static_cast<uint32_t>(header.PayloadLength()))
    {}

    void WebSocketServerConnectionObserver::ReceivingStateReceiveData::DataReceived()
    {
        ReceiveData();

        if (sizeToReceive == 0)
            SetNextState();
    }
    
    void WebSocketServerConnectionObserver::ReceivingStateReceiveData::ReceiveData()
    {
        infra::SharedPtr<infra::StreamReader> reader = connection.services::ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        while (!stream.Empty() && !connection.receiveBuffer.full() && sizeToReceive != 0)
            ConsumeData(stream);

        connection.services::ConnectionObserver::Subject().AckReceived();
    }

    void WebSocketServerConnectionObserver::ReceivingStateReceiveData::ConsumeData(infra::DataInputStream& stream)
    {
        if (header.OpCode() == services::WebSocketOpCode::opCodeClose)
            DiscardRest(stream);
        else if (header.OpCode() == services::WebSocketOpCode::opCodePing)
        {
            auto startSize = connection.pongBuffer.size();

            auto growSize = std::min(connection.pongBuffer.size() + std::min<std::size_t>(stream.Available(), sizeToReceive), connection.pongBuffer.max_size()) - connection.pongBuffer.size();
            connection.pongBuffer.resize(connection.pongBuffer.size() + growSize);
            sizeToReceive -= growSize;
            stream >> infra::Tail(infra::MakeRange(connection.pongBuffer), growSize);

            if (connection.pongBuffer.full())
                DiscardRest(stream);

            ProcessPongData(startSize);
        }
        else
        {
            auto startSize = connection.receiveBuffer.size();

            auto growSize = std::min(connection.receiveBuffer.size() + std::min<std::size_t>(stream.Available(), sizeToReceive), connection.receiveBuffer.max_size()) - connection.receiveBuffer.size();
            connection.receiveBuffer.resize(connection.receiveBuffer.size() + growSize);
            sizeToReceive -= growSize;

            while (growSize != 0)
            {
                auto range = connection.receiveBuffer.contiguous_range(connection.receiveBuffer.end() - growSize);
                stream >> range;
                growSize -= range.size();
            }

            ProcessFrameData(startSize);
        }
    }

    void WebSocketServerConnectionObserver::ReceivingStateReceiveData::DiscardRest(infra::DataInputStream& stream)
    {
        while (true)
        {
            auto discardSize = std::min<std::size_t>(stream.Available(), sizeToReceive);

            if (discardSize == 0)
                break;

            auto range = stream.ContiguousRange(discardSize);
            sizeToReceive -= range.size();
        }
    }

    void WebSocketServerConnectionObserver::ReceivingStateReceiveData::ProcessFrameData(uint32_t alreadyReceived)
    {
        for (std::size_t i = alreadyReceived; i != connection.receiveBuffer.size(); ++i, ++receiveOffset)
            connection.receiveBuffer[i] = connection.receiveBuffer[i] ^ header.MaskingKey()[receiveOffset % 4];
    }

    void WebSocketServerConnectionObserver::ReceivingStateReceiveData::ProcessPongData(uint32_t alreadyReceived)
    {
        for (std::size_t i = alreadyReceived; i != connection.pongBuffer.size(); ++i, ++receiveOffset)
            connection.pongBuffer[i] = connection.pongBuffer[i] ^ header.MaskingKey()[receiveOffset % 4];
    }

    void WebSocketServerConnectionObserver::ReceivingStateReceiveData::SetNextState()
    {
        if (header.OpCode() == services::WebSocketOpCode::opCodeClose)
            connection.SetReceivingStateClose();
        else if (header.OpCode() == services::WebSocketOpCode::opCodePing)
            connection.SetReceivingStatePong();
        else
            connection.SetReceivingStateReceiveHeader();
    }

    WebSocketServerConnectionObserver::ReceivingStateClose::ReceivingStateClose(WebSocketServerConnectionObserver& connection)
        : ReceivingStateThatSendsData(connection)
        , connection(connection)
    {
        connection.receiveBuffer.clear();
    }

    void WebSocketServerConnectionObserver::ReceivingStateClose::Send(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        connection.SendFrame(services::WebSocketOpCode::opCodeClose, infra::ConstByteRange(), *writer);
        writer = nullptr;
        connection.Close();
    }

    WebSocketServerConnectionObserver::ReceivingStatePong::ReceivingStatePong(WebSocketServerConnectionObserver& connection)
        : ReceivingStateThatSendsData(connection)
        , connection(connection)
    {}

    void WebSocketServerConnectionObserver::ReceivingStatePong::Send(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        connection.SendFrame(services::WebSocketOpCode::opCodePong, infra::MakeRange(connection.pongBuffer), *writer);
        connection.pongBuffer.clear();
        connection.SetReceivingStateReceiveHeader();
    }

    void WebSocketServerConnectionObserver::SendingState::CheckForSomethingToDo()
    {}

    WebSocketServerConnectionObserver::SendingStateIdle::SendingStateIdle(WebSocketServerConnectionObserver& connection)
        : connection(connection)
    {}

    void WebSocketServerConnectionObserver::SendingStateIdle::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        std::abort();
    }

    void WebSocketServerConnectionObserver::SendingStateIdle::CheckForSomethingToDo()
    {
        if (connection.receivingStateThatWantsToSendData != nullptr)
            connection.sendingState.Emplace<SendingStateInternalData>(connection);
        else if (connection.sendBufferReadyForSending)
            connection.sendingState.Emplace<SendingStateExternalData>(connection);
        else
            connection.TryAllocateSendStream();
    }

    WebSocketServerConnectionObserver::SendingStateInternalData::SendingStateInternalData(WebSocketServerConnectionObserver& connection)
        : connection(connection)
    {
        connection.services::ConnectionObserver::Subject().RequestSendStream(connection.services::ConnectionObserver::Subject().MaxSendStreamSize());
    }

    void WebSocketServerConnectionObserver::SendingStateInternalData::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::WeakPtr<void> self(connection.services::ConnectionObserver::Subject().Observer());

        infra::PostAssign(connection.receivingStateThatWantsToSendData, nullptr)->Send(std::move(writer));

        if (self.lock() != nullptr)
        {
            writer = nullptr;

            connection.SetStateSendingIdle();
        }
    }

    WebSocketServerConnectionObserver::SendingStateExternalData::SendingStateExternalData(WebSocketServerConnectionObserver& connection)
        : connection(connection)
    {
        connection.sendBufferReadyForSending = false;
        connection.services::ConnectionObserver::Subject().RequestSendStream(connection.services::ConnectionObserver::Subject().MaxSendStreamSize());
    }

    void WebSocketServerConnectionObserver::SendingStateExternalData::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        connection.SendFrame(services::WebSocketOpCode::opCodeBin, infra::MakeRange(connection.sendBuffer), *writer);
        connection.sendBuffer.clear();
        writer = nullptr;

        connection.SetStateSendingIdle();
    }
}
