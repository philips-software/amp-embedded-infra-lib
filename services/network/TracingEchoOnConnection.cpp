#include "services/network/TracingEchoOnConnection.hpp"
#include "infra/stream/ByteInputStream.hpp"

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

    // void TracingEchoOnConnection::ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, infra::StreamReaderWithRewinding& reader)
    //{
    //     auto service = FindService(serviceId);

    //    if (service != nullptr)
    //    {
    //        auto marker = reader.ConstructSaveMarker();

    //        infra::DataInputStream::WithErrorPolicy stream(reader, infra::noFail);
    //        infra::ProtoLengthDelimited contentsCopy(stream, stream.ErrorPolicy(), contents.Available());

    //        tracer.Trace() << "< ";
    //        service->TraceMethod(methodId, contentsCopy, tracer);

    //        reader.Rewind(marker);
    //    }
    //    else
    //        tracer.Trace() << "< Unknown service " << serviceId << " method " << methodId;

    //    EchoOnConnection::ExecuteMethod(serviceId, methodId, contents, reader);
    //}

    // void TracingEchoOnConnection::SetStreamWriter(infra::SharedPtr<infra::StreamWriter>&& writer)
    //{
    //     EchoOnConnection::SetStreamWriter(tracingWriter.Emplace(std::move(writer), *this));
    // }

    void TracingEchoOnConnection::SendingData(infra::ConstByteRange range) const
    {
        infra::ByteInputStream stream(range, infra::noFail);
        infra::StreamErrorPolicy formatErrorPolicy(infra::softFail);
        infra::ProtoParser parser(stream, formatErrorPolicy);
        auto serviceId = static_cast<uint32_t>(parser.GetVarInt());
        auto message = parser.GetField();
        if (stream.Failed() || formatErrorPolicy.Failed() || !message.first.Is<infra::ProtoLengthDelimited>())
        {
            tracer.Trace() << "> Malformed message";
            return;
        }

        SendingMethod(serviceId, message.second, message.first.Get<infra::ProtoLengthDelimited>());
        if (stream.Failed() || formatErrorPolicy.Failed())
            tracer.Continue() << "... Malformed message";
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
    {}

    TracingEchoOnConnection::TracingWriter::~TracingWriter()
    {
        echo.SendingData(infra::MakeRange(writer.Storage()));
    }

    void TracingEchoOnConnection::TracingWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        writer.Insert(range, errorPolicy);
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
