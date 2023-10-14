#include "services/network/TracingEchoOnConnection.hpp"
#include "infra/stream/BoundedVectorInputStream.hpp"

namespace services
{
    TracingEchoOnConnection::TracingEchoOnConnection(services::Tracer& tracer, EchoErrorPolicy& errorPolicy)
        : EchoOnConnection(errorPolicy)
        , tracer(tracer)
    {}

    void TracingEchoOnConnection::AddServiceTracer(ServiceTracer& service)
    {
        services.push_front(service);
    }

    void TracingEchoOnConnection::RemoveServiceTracer(ServiceTracer& service)
    {
        services.erase_slow(service);
    }

    infra::SharedPtr<MethodSerializer> TracingEchoOnConnection::GrantSend(ServiceProxy& proxy)
    {
        writerBuffer.clear();
        serializer = EchoOnConnection::GrantSend(proxy);
        return infra::MakeContainedSharedObject(static_cast<MethodSerializer&>(*this), serializer);
    }

    infra::SharedPtr<MethodDeserializer> TracingEchoOnConnection::StartingMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const infra::SharedPtr<MethodDeserializer>& deserializer)
    {
        receivingService = FindService(serviceId);

        if (receivingService != nullptr)
        {
            receivingMethodId = methodId;

            readerBuffer.clear();
            this->deserializer = deserializer;
            return infra::MakeContainedSharedObject(static_cast<MethodDeserializer&>(*this), deserializer);
        }
        else
        {
            tracer.Trace() << "< Unknown service " << serviceId << " method " << methodId;
            return deserializer;
        }
    }

    bool TracingEchoOnConnection::Serialize(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        return serializer->Serialize(tracingWriter.Emplace(std::move(writer), *this));
    }

    void TracingEchoOnConnection::SerializationDone()
    {
        infra::BoundedVectorInputStream stream(writerBuffer, infra::noFail);
        infra::StreamErrorPolicy formatErrorPolicy(infra::softFail);
        infra::ProtoParser parser(stream, formatErrorPolicy);
        auto serviceId = static_cast<uint32_t>(parser.GetVarInt());
        auto message = parser.GetField();
        if (stream.Failed())
            tracer.Trace() << "> message too big";
        else if (formatErrorPolicy.Failed() || !message.first.Is<infra::ProtoLengthDelimited>())
            tracer.Trace() << "> Malformed message";
        else
        {
            SendingMethod(serviceId, message.second, message.first.Get<infra::ProtoLengthDelimited>());
            if (stream.Failed() || formatErrorPolicy.Failed())
                tracer.Continue() << "... Malformed message";
        }
    }

    void TracingEchoOnConnection::MethodContents(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader)
    {
        auto start = reader->ConstructSaveMarker();
        while (!reader->Empty() && !readerBuffer.full())
        {
            auto range = reader->ExtractContiguousRange(readerBuffer.max_size() - readerBuffer.size());
            readerBuffer.insert(readerBuffer.end(), range.begin(), range.end());
        }
        reader->Rewind(start);

        deserializer->MethodContents(reader);
    }

    void TracingEchoOnConnection::ExecuteMethod()
    {
        infra::BoundedVectorInputStream stream(readerBuffer, infra::noFail);
        infra::ProtoLengthDelimited contentsCopy(stream, stream.ErrorPolicy(), stream.Available());

        tracer.Trace() << "< ";
        receivingService->TraceMethod(receivingMethodId, contentsCopy, tracer);

        deserializer->ExecuteMethod();
    }

    bool TracingEchoOnConnection::Failed() const
    {
        return deserializer->Failed();
    }

    void TracingEchoOnConnection::SendingMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents) const
    {
        auto service = FindService(serviceId);

        if (service != nullptr)
        {
            tracer.Trace() << "> ";
            service->TraceMethod(methodId, contents, tracer);
        }
        else
            tracer.Trace() << "> Unknown service " << serviceId << " method " << methodId;
    }

    const ServiceTracer* TracingEchoOnConnection::FindService(uint32_t serviceId) const
    {
        for (auto& service : services)
            if (service.ServiceId() == serviceId)
                return &service;

        return nullptr;
    }

    TracingEchoOnConnection::TracingWriter::TracingWriter(infra::SharedPtr<infra::StreamWriter>&& delegate, TracingEchoOnConnection& echo)
        : echo(echo)
        , delegate(std::move(delegate))
        , writer(echo.writerBuffer)
    {}

    void TracingEchoOnConnection::TracingWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        infra::StreamErrorPolicy tracingErrorPolicy(infra::noFail);
        writer.Insert(range, tracingErrorPolicy);
        delegate->Insert(range, errorPolicy);
    }

    std::size_t TracingEchoOnConnection::TracingWriter::Available() const
    {
        return delegate->Available();
    }

    std::size_t TracingEchoOnConnection::TracingWriter::ConstructSaveMarker() const
    {
        return delegate->ConstructSaveMarker();
    }

    std::size_t TracingEchoOnConnection::TracingWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return delegate->GetProcessedBytesSince(marker);
    }

    infra::ByteRange TracingEchoOnConnection::TracingWriter::SaveState(std::size_t marker)
    {
        savedRange = writer.SaveState(writer.ConstructSaveMarker() - (delegate->ConstructSaveMarker() - marker));
        savedDelegateRange = delegate->SaveState(marker);
        return savedDelegateRange;
    }

    void TracingEchoOnConnection::TracingWriter::RestoreState(infra::ByteRange range)
    {
        auto restoreSize = savedDelegateRange.size() - range.size();
        auto from = infra::ByteRange(range.begin() - restoreSize, range.begin());

        infra::Copy(from, infra::Head(savedRange, restoreSize));
        writer.RestoreState(infra::DiscardHead(savedRange, restoreSize));

        savedRange = infra::ByteRange();
        delegate->RestoreState(range);
    }

    infra::ByteRange TracingEchoOnConnection::TracingWriter::Overwrite(std::size_t marker)
    {
        std::abort();
    }
}
