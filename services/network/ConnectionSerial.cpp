#include "services/network/ConnectionSerial.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/Endian.hpp"

namespace services
{
    ConnectionSerial::ConnectionSerial(infra::ByteRange sendBuffer, infra::ByteRange parseBuffer, infra::BoundedDeque<uint8_t>& receivedDataQueue, hal::SerialCommunication& serialCommunication, infra::Function<void()> onConnected, infra::Function<void()> onDisconnected, size_t minUpdateSize)
        : receivedDataQueue(receivedDataQueue)
        , sendBuffer(sendBuffer)
        , serialCommunication(serialCommunication)
        , receiveQueue(parseBuffer, [this]() { DataReceived(); })
        , state(infra::InPlaceType<StateInitSizeRequest>(), *this)
        , minUpdateSize(std::max(minUpdateSize, MessageHeader::HeaderSize + 1))
        , onConnected(onConnected)
        , onDisconnected(onDisconnected)
    {
        assert(minUpdateSize <= receiveQueue.EmptySize() - MessageHeader::HeaderSize);

        serialCommunication.ReceiveData([this](infra::ConstByteRange data) {
            for (uint8_t byte : data)
                if (!receiveQueue.Full())
                    receiveQueue.AddFromInterrupt(byte);
                else
                    abort();
        });
    }

    void ConnectionSerial::AllocatedSendStream()
    {
        sendStreamAvailable = false;
        pendingSendStream = 0;

        infra::EventDispatcher::Instance().Schedule([this]() {
            infra::SharedPtr<StreamWriterWithCyclicBuffer> writer = sendStream.Emplace(sendBuffer, *this);
            this->Observer().SendStreamAvailable(std::move(writer));
        });
    }

    void ConnectionSerial::NotifyObserverDataReceived()
    {
        if (!observerNotified)
        {
            observerNotified = true;
            Observer().DataReceived();
        }
    }

    size_t ConnectionSerial::ReceiveBufferSize() const
    {
        return receiveQueue.EmptySize();
    }

    void ConnectionSerial::RequestSendStream(std::size_t sendSize)
    {
        assert(sendSize <= MaxSendStreamSize());

        if (sendStreamAvailable)
            AllocatedSendStream();
        else
            pendingSendStream = sendSize;
    }

    std::size_t ConnectionSerial::MaxSendStreamSize() const
    {
        return sendBuffer.size();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ConnectionSerial::ReceiveStream()
    {
        observerNotified = false;
        receivedDataReader.Emplace(receivedDataQueue);
        return infra::UnOwnedSharedPtr(*receivedDataReader);
    }

    void ConnectionSerial::AckReceived()
    {
        state->AckReceived();
    }

    void ConnectionSerial::CloseAndDestroy()
    {
        std::abort();
    }

    void ConnectionSerial::AbortAndDestroy()
    {
        std::abort();
    }

    void ConnectionSerial::TryProcessHeader()
    {
        if (!messageHeader->IsDone())
            return;

        if (messageHeader->IsFailed())
            state->Reset();
        else
            ProcessHeader();

        messageHeader = infra::none;
    }

    void ConnectionSerial::DataReceived()
    {
        while (!receiveQueue.Empty())
        {
            if (contentToReceive > 0)
                receiveQueue.Consume(state->ContentReceived(infra::Head(receiveQueue.ContiguousRange(), std::min(contentToReceive, receiveQueue.Size()))));
            else
            {
                TryConstructHeader();
                TryProcessHeader();
            }

            state->EvaluatePendingSend();
        }
    }

    void ConnectionSerial::TryConstructHeader()
    {
        if (messageHeader == infra::none)
            messageHeader.Emplace();

        while (!receiveQueue.Empty() && !messageHeader->IsDone())
        {
            messageHeader->AddByte(receiveQueue[0]);
            receiveQueue.Consume(1);
            state->AccumulateWindowUpdateSize(1);
        }
    }

    void ConnectionSerial::ProcessHeader()
    {
        switch (messageHeader->Type())
        {
        case MessageType::SizeRequest:
            state->SizeRequestReceived();
            break;

        case MessageType::SizeResponseRequest:
            state->SizeResponseRequestReceived(messageHeader->Size());
            break;

        case MessageType::SizeResponse:
            state->SizeResponseReceived(messageHeader->Size());
            break;

        case MessageType::Content:
            contentToReceive = messageHeader->Size();
            break;

        case MessageType::SizeUpdate:
            state->SizeUpdateReceived(messageHeader->Size());
            break;
        }
    }

    void ConnectionSerial::InitCompleted()
    {
        GoStateConnectedIdle();
        onConnected();
    }

    void ConnectionSerial::ResetConnection()
    {
        onDisconnected();
        sendStreamAvailable = true;
        GoStateInitSizeRequest();
    }

    void ConnectionSerial::SendStreamAllocatable()
    {
        sendStreamAvailable = true;

        if (pendingSendStream != 0)
            AllocatedSendStream();
    }

    void ConnectionSerial::SendStreamFilled(infra::ByteRange data)
    {
        contentReadyForSend = data;
        infra::EventDispatcher::Instance().Schedule([this]() { state->EvaluatePendingSend(); });
    }

    void ConnectionSerial::GoStateInitSizeRequest()
    {
        state.Emplace<StateInitSizeRequest>(*this);
    }

    void ConnectionSerial::GoStateInitSizeResponseRequest()
    {
        state.Emplace<StateInitSizeResponseRequest>(*this);
    }

    void ConnectionSerial::GoStateInitSizeResponse()
    {
        state.Emplace<StateInitSizeResponse>(*this);
    }

    void ConnectionSerial::GoStateConnectedIdle()
    {
        state.Emplace<StateConnectedIdle>(*this);
    }

    void ConnectionSerial::GoStateConnectedSendingContent(size_t size)
    {
        state.Emplace<StateConnectedSendingContent>(*this, size);
    }

    void ConnectionSerial::GoStateSendingUpdate()
    {
        state.Emplace<StateSendingUpdate>(*this);
    }

    ConnectionSerial::MessageHeader::MessageHeader()
    {}

    ConnectionSerial::MessageHeader::MessageHeader(MessageType type)
    {
        headerContent[0] = static_cast<uint8_t>(type);
        if (type == MessageType::SizeRequest)
        {
            headerContent[1] = headerContent[2] = static_cast<uint8_t>(MessageType::SizeRequest);
            isComplete = true;
        }
    }

    ConnectionSerial::MessageType ConnectionSerial::MessageHeader::Type()
    {
        return static_cast<MessageType>(headerContent[0]);
    }

    size_t ConnectionSerial::MessageHeader::Size()
    {
        assert(Type() != MessageType::SizeRequest);
        return static_cast<uint16_t>(headerContent[1]) << 8 | static_cast<uint16_t>(headerContent[2]);
    }

    bool ConnectionSerial::MessageHeader::IsDone()
    {
        return isComplete || isFailed;
    }

    bool ConnectionSerial::MessageHeader::IsFailed()
    {
        return isFailed;
    }

    void ConnectionSerial::MessageHeader::AddByte(uint8_t byte)
    {
        if (IsDone())
            return;

        switch (byteToSet)
        {
        case 0:
            if (!IsHeaderByte(byte))
                isFailed = true;
            else
                headerContent[0] = byte;
            break;

        case 1:
            if (Type() != MessageType::SizeRequest)
                SetContent(static_cast<uint16_t>(byte << 8));
            else if (static_cast<MessageType>(byte) != MessageType::SizeRequest)
                isFailed = true;
            break;

        case 2:
            if (Type() != MessageType::SizeRequest)
                SetContent(static_cast<uint16_t>(byte + Size()));
            else if (static_cast<MessageType>(byte) != MessageType::SizeRequest)
                isFailed = true;

            isComplete = true;
            break;
        }

        ++byteToSet;
    }

    void ConnectionSerial::MessageHeader::SetSize(size_t size)
    {
        SetContent(static_cast<uint16_t>(size));
        isComplete = true;
    }

    void ConnectionSerial::MessageHeader::SetContent(uint16_t content)
    {
        infra::Copy(infra::MakeByteRange(infra::BigEndian<uint16_t>(content)), infra::Tail(infra::MakeRange(headerContent), 2));
    }

    infra::ConstByteRange ConnectionSerial::MessageHeader::Range()
    {
        return infra::MakeByteRange(headerContent);
    }

    bool ConnectionSerial::MessageHeader::IsHeaderByte(uint8_t byte)
    {
        return byte >= static_cast<uint8_t>(MessageType::SizeRequest) && byte <= static_cast<uint8_t>(MessageType::SizeUpdate);
    }

    bool ConnectionSerial::MessageHeader::IsEscapeByte(uint8_t byte)
    {
        return byte == static_cast<uint8_t>(MessageType::Escape);
    }

    ConnectionSerial::State::State(ConnectionSerial& connection)
        : connection(connection)
        , serialCommunication(connection.serialCommunication)
    {}

    void ConnectionSerial::State::AckReceived()
    {}

    ConnectionSerial::StateInit::StateInit(ConnectionSerial& connection)
        : State(connection)
    {}

    void ConnectionSerial::StateInit::Reset()
    {
        if (!sendInProgress)
            connection.GoStateInitSizeRequest();
        else
            resetAfterSend = true;
    }

    void ConnectionSerial::StateInit::SizeResponseReceived(size_t size)
    {}

    void ConnectionSerial::StateInit::SizeResponseRequestReceived(size_t size)
    {}

    size_t ConnectionSerial::StateInit::ContentReceived(infra::ConstByteRange receivedRange)
    {
        connection.contentToReceive = 0;
        return 1;
    }

    void ConnectionSerial::StateInit::SizeUpdateReceived(size_t size)
    {
        Reset();
    }

    void ConnectionSerial::StateInit::AccumulateWindowUpdateSize(size_t size)
    {}

    void ConnectionSerial::StateInitSizeRequest::SendWindowSizeRequest()
    {
        sendInProgress = true;
        serialCommunication.SendData(windowSizeReqMsg.Range(), [this]() {
            sendInProgress = false;

            if (!responseRequestAfterSend && !responseAfterSend)
                windowRequestTimer.Start(std::chrono::seconds(1), [this]() { SendWindowSizeRequest(); });
            else if (responseRequestAfterSend)
                connection.GoStateInitSizeResponseRequest();
            else
                connection.GoStateInitSizeResponse();
        });
    }

    void ConnectionSerial::StateInit::EvaluatePendingSend()
    {}

    ConnectionSerial::StateInitSizeRequest::StateInitSizeRequest(ConnectionSerial& connection)
        : StateInit(connection)
    {
        SendWindowSizeRequest();
    }

    void ConnectionSerial::StateInitSizeRequest::SizeResponseRequestReceived(size_t size)
    {
        connection.peerBufferSize = size;

        if (sendInProgress)
            responseAfterSend = true;
        else
            connection.GoStateInitSizeResponse();
    }

    void ConnectionSerial::StateInitSizeRequest::Reset()
    {}

    void ConnectionSerial::StateInitSizeRequest::SizeRequestReceived()
    {
        if (sendInProgress)
            responseRequestAfterSend = true;
        else
            connection.GoStateInitSizeResponseRequest();
    }

    ConnectionSerial::StateInitSizeResponseRequest::StateInitSizeResponseRequest(ConnectionSerial& connection)
        : StateInit(connection)
    {
        SendWindowSizeResponseRequest();
    }

    void ConnectionSerial::StateInitSizeResponseRequest::SendWindowSizeResponseRequest()
    {
        sendInProgress = true;
        windowSizeRespReqMsg.SetSize(connection.ReceiveBufferSize());
        serialCommunication.SendData(windowSizeRespReqMsg.Range(), [this]() {
            sendInProgress = false;

            if (responseRequestAfterSend)
                connection.GoStateInitSizeResponseRequest();
        });
    }

    void ConnectionSerial::StateInitSizeResponseRequest::SizeResponseReceived(size_t size)
    {
        connection.peerBufferSize = size;
        connection.InitCompleted();
    }

    void ConnectionSerial::StateInitSizeResponseRequest::SizeRequestReceived()
    {
        if (!sendInProgress)
            connection.GoStateInitSizeResponseRequest();
        else
            responseRequestAfterSend = true;
    }

    ConnectionSerial::StateInitSizeResponse::StateInitSizeResponse(ConnectionSerial& connection)
        : StateInit(connection)
    {
        SendWindowSizeResponse();
    }

    void ConnectionSerial::StateInitSizeResponse::SendWindowSizeResponse()
    {
        windowSizeRespMsg.SetSize(connection.ReceiveBufferSize());
        serialCommunication.SendData(windowSizeRespMsg.Range(), [this]() {
            if (responseRequestAfterSend)
                connection.GoStateInitSizeResponseRequest();
            else
                connection.InitCompleted();
        });
    }

    void ConnectionSerial::StateInitSizeResponse::SizeRequestReceived()
    {
        responseRequestAfterSend = true;
    }

    ConnectionSerial::StateConnected::StateConnected(ConnectionSerial& connection)
        : State(connection)
        , sendBuffer(connection.contentReadyForSend)
    {}

    void ConnectionSerial::StateConnected::Reset()
    {
        sendBuffer.clear();
        connection.ResetConnection();
    }

    bool ConnectionSerial::StateConnected::IsEscapedByte(uint8_t byte) const
    {
        return MessageHeader::IsEscapeByte(byte) || MessageHeader::IsHeaderByte(byte);
    }

    size_t ConnectionSerial::StateConnected::ContentSizeToSend() const
    {
        if (connection.totalContentSizeToSend != 0)
            return 0;

        size_t windowUpdateAndHeaderRoom = 2 * MessageHeader::HeaderSize;
        size_t maxContentSize = connection.peerBufferSize > windowUpdateAndHeaderRoom ? connection.peerBufferSize - windowUpdateAndHeaderRoom : 0;

        size_t contentSize = 0;
        for (size_t i = 0; i < sendBuffer.size(); i++)
        {
            uint8_t incr = 0;

            if (IsEscapedByte(sendBuffer[i]))
                ++incr;

            if (++incr + contentSize >= maxContentSize)
                return maxContentSize;

            contentSize += incr;
        }

        return contentSize;
    }

    void ConnectionSerial::StateConnectedSendingContent::SendContentWithSize(size_t size)
    {
        connection.totalContentSizeToSend = size;
        contentMsgHeader.SetSize(size);
        connection.peerBufferSize -= size + MessageHeader::HeaderSize;

        serialCommunication.SendData(contentMsgHeader.Range(), [this]() {
            SendEscapedContent();
        });
    }

    void ConnectionSerial::StateConnectedSendingContent::SendEscapedContent()
    {
        size_t firstEscapedByte = 0;
        while (firstEscapedByte < connection.totalContentSizeToSend && !IsEscapedByte(sendBuffer[firstEscapedByte]))
            firstEscapedByte++;

        if (firstEscapedByte == 0)
        {
            escapedRange[1] = sendBuffer.front();
            SendPartialContent(escapedRange, 1);
        }
        else
        {
            infra::ByteRange firstEscapeLessRange = infra::Head(sendBuffer, firstEscapedByte);
            SendPartialContent(firstEscapeLessRange, firstEscapeLessRange.size());
        }
    }

    void ConnectionSerial::StateConnectedSendingContent::SendPartialContent(infra::ByteRange range, size_t rawContentSize)
    {
        partialContentSizeBeingSent = range.size();
        serialCommunication.SendData(range, [this, rawContentSize]() {
            sendBuffer = infra::Tail(sendBuffer, sendBuffer.size() - rawContentSize);
            connection.totalContentSizeToSend -= partialContentSizeBeingSent;

            if (connection.totalContentSizeToSend == 0)
            {
                if (sendBuffer.empty())
                    connection.SendStreamAllocatable();

                if (!resetAfterSend)
                    GoIdle();
                else
                    StateConnected::Reset();
            }
            else
                SendEscapedContent();
        });
    }

    void ConnectionSerial::StateConnected::EvaluatePendingSend()
    {
        if (UpdatePossible())
            connection.GoStateSendingUpdate();
        else
        {
            size_t contentSize = ContentSizeToSend();
            if (contentSize != 0)
                connection.GoStateConnectedSendingContent(contentSize);
        }
    }

    bool ConnectionSerial::StateConnected::UpdatePossible()
    {
        return connection.accumulatedWindowSizeUpdate.ValueOr(0) >= connection.minUpdateSize &&
            !connection.updateInProgress && connection.peerBufferSize >= MessageHeader::HeaderSize;
    }

    void ConnectionSerial::StateConnected::AccumulateWindowUpdateSize(size_t size)
    {
        if (connection.accumulatedWindowSizeUpdate)
            *connection.accumulatedWindowSizeUpdate += size;
        else
            connection.accumulatedWindowSizeUpdate.Emplace(size);
    }

    void ConnectionSerial::StateConnected::AckReceived()
    {
        size_t bytesConsumed = connection.receivedDataReader->ConsumeExtracted();
        AccumulateWindowUpdateSize(bytesConsumed);
        EvaluatePendingSend();
    }

    void ConnectionSerial::StateConnected::SizeRequestReceived()
    {
        Reset();
    }

    void ConnectionSerial::StateConnected::SizeResponseRequestReceived(size_t size)
    {
        Reset();
    }

    void ConnectionSerial::StateConnected::SizeResponseReceived(size_t size)
    {
        Reset();
    }

    size_t ConnectionSerial::StateConnected::CopyEscapedContent(infra::ConstByteRange receivedRange)
    {
        size_t copiedAmount = 0;
        for (; copiedAmount != receivedRange.size(); ++copiedAmount)
        {
            if (MessageHeader::IsEscapeByte(receivedRange[copiedAmount]) && !connection.isEscapeSeen)
            {
                AccumulateWindowUpdateSize(1);
                connection.isEscapeSeen = true;
            }
            else if (MessageHeader::IsHeaderByte(receivedRange[copiedAmount]) && !connection.isEscapeSeen)
                break;
            else
            {
                connection.receivedDataQueue.push_back(receivedRange[copiedAmount]);
                connection.isEscapeSeen = false;
            }
        }

        return copiedAmount;
    }

    size_t ConnectionSerial::StateConnected::ContentReceived(infra::ConstByteRange receivedRange)
    {
        size_t copiedAmount = CopyEscapedContent(receivedRange);

        if (copiedAmount > 0) //What if we received only an escape byte?
            connection.NotifyObserverDataReceived();

        if (copiedAmount == receivedRange.size())
            connection.contentToReceive -= copiedAmount;
        else
        {
            connection.contentToReceive = 0;
            Reset();
        }

        return copiedAmount;
    }

    void ConnectionSerial::StateConnected::SizeUpdateReceived(size_t size)
    {
        connection.peerBufferSize += size;
    }

    ConnectionSerial::StateConnectedIdle::StateConnectedIdle(ConnectionSerial& connection)
        : StateConnected(connection)
    {
        EvaluatePendingSend();
    }

    ConnectionSerial::StateSending::StateSending(ConnectionSerial& connection)
        : StateConnected(connection)
    {}

    void ConnectionSerial::StateSending::EvaluatePendingSend()
    {}

    void ConnectionSerial::StateSending::Reset()
    {
        resetAfterSend = true;
    }

    void ConnectionSerial::StateSending::GoIdle()
    {
        connection.GoStateConnectedIdle();
    }

    ConnectionSerial::StateSendingUpdate::StateSendingUpdate(ConnectionSerial& connection)
        : StateSending(connection)
    {
        SendUpdate();
    }

    void ConnectionSerial::StateSendingUpdate::SendUpdate()
    {
        connection.updateInProgress = true;
        connection.peerBufferSize -= MessageHeader::HeaderSize;

        updateMsg.SetSize(*connection.accumulatedWindowSizeUpdate);
        *connection.accumulatedWindowSizeUpdate = 0;

        serialCommunication.SendData(updateMsg.Range(), [this]() {
            connection.updateInProgress = false;

            if (*connection.accumulatedWindowSizeUpdate == 0)
                connection.accumulatedWindowSizeUpdate = infra::none;

            if (!resetAfterSend)
                GoIdle();
            else
                StateConnected::Reset();
        });
    }

    ConnectionSerial::StateConnectedSendingContent::StateConnectedSendingContent(ConnectionSerial& connection, size_t contentSize)
        : StateSending(connection)
    {
        SendContentWithSize(contentSize);
    }

    ConnectionSerial::StreamWriterWithCyclicBuffer::StreamWriterWithCyclicBuffer(infra::ByteRange& buffer, ConnectionSerial& connection)
        : infra::ByteOutputStreamWriter(buffer)
        , connection(connection)
    {}

    ConnectionSerial::StreamWriterWithCyclicBuffer::~StreamWriterWithCyclicBuffer()
    {
        connection.SendStreamFilled(Processed());
    }

    ConnectionSerial::SerialConnectionStreamReader::SerialConnectionStreamReader(infra::BoundedDeque<uint8_t>& receivedDataQueue)
        : receivedDataQueue(receivedDataQueue)
    {}

    std::size_t ConnectionSerial::SerialConnectionStreamReader::ConstructSaveMarker() const
    {
        std::abort();
    }

    void ConnectionSerial::SerialConnectionStreamReader::Rewind(std::size_t marker)
    {
        std::abort();
    }

    void ConnectionSerial::SerialConnectionStreamReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(Available() >= range.size());

        if (errorPolicy.Failed())
            return;

        infra::ConstByteRange extractedRange1 = ExtractContiguousRange(range.size());
        infra::Copy(extractedRange1, infra::Head(range, extractedRange1.size()));

        if (extractedRange1.size() < range.size())
        {
            size_t diff = range.size() - extractedRange1.size();
            infra::ConstByteRange extractedRange2 = ExtractContiguousRange(diff);
            infra::Copy(extractedRange2, infra::Tail(range, diff));
        }
    }

    uint8_t ConnectionSerial::SerialConnectionStreamReader::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(Available() > 0);

        if (errorPolicy.Failed())
            return 0;

        return receivedDataQueue[extractedAmount];
    }

    infra::ConstByteRange ConnectionSerial::SerialConnectionStreamReader::ExtractContiguousRange(std::size_t max)
    {
        infra::ByteRange range = infra::Head(receivedDataQueue.contiguous_range(receivedDataQueue.begin() + extractedAmount), max);
        extractedAmount += range.size();

        return range;
    }

    infra::ConstByteRange ConnectionSerial::SerialConnectionStreamReader::PeekContiguousRange(std::size_t start)
    {
        if (start >= receivedDataQueue.size() - extractedAmount)
            return infra::ConstByteRange();

        return receivedDataQueue.contiguous_range(receivedDataQueue.begin() + extractedAmount + start);
    }

    bool ConnectionSerial::SerialConnectionStreamReader::Empty() const
    {
        return Available() == 0;
    }

    size_t ConnectionSerial::SerialConnectionStreamReader::Available() const
    {
        return receivedDataQueue.size() - extractedAmount;
    }

    size_t ConnectionSerial::SerialConnectionStreamReader::ConsumeExtracted()
    {
        for (size_t i = 0; i < extractedAmount; i++)
            receivedDataQueue.pop_front();

        return extractedAmount;
    }
}
