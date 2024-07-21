#include "protobuf/echo/TracingEcho.hpp"
#include "infra/stream/BoundedVectorInputStream.hpp"
#include "infra/stream/ByteInputStream.hpp"

namespace services
{
    ServiceTracer::ServiceTracer(uint32_t serviceId)
        : serviceId(serviceId)
    {}

    uint32_t ServiceTracer::ServiceId() const
    {
        return serviceId;
    }

    void PrintField(bool value, services::Tracer& tracer)
    {
        tracer.Continue() << (value ? "true" : "false");
    }

    void PrintField(uint32_t value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(int32_t value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(uint64_t value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(int64_t value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(const infra::BoundedConstString& value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(const infra::BoundedVector<uint8_t>& value, services::Tracer& tracer)
    {
        tracer.Continue() << "[";
        for (auto v : value)
            tracer.Continue() << infra::hex << infra::Width(2, 0) << v;
        tracer.Continue() << "]";
    }

    namespace detail
    {
        TracingEchoOnStreamsDescendantHelper::TracingEchoOnStreamsDescendantHelper(services::Tracer& tracer)
            : tracer(tracer)
        {}

        void TracingEchoOnStreamsDescendantHelper::AddServiceTracer(ServiceTracer& service)
        {
            services.push_front(service);
        }

        void TracingEchoOnStreamsDescendantHelper::RemoveServiceTracer(ServiceTracer& service)
        {
            services.erase_slow(service);
        }

        infra::SharedPtr<MethodSerializer> TracingEchoOnStreamsDescendantHelper::GrantSend(const infra::SharedPtr<MethodSerializer>& serializer)
        {
            this->serializer = serializer;
            return serializerAccess.MakeShared(static_cast<MethodSerializer&>(*this));
        }

        infra::SharedPtr<MethodDeserializer> TracingEchoOnStreamsDescendantHelper::StartingMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, infra::SharedPtr<MethodDeserializer>&& deserializer)
        {
            receivingService = FindService(serviceId);

            if (receivingService != nullptr)
            {
                receivingMethodId = methodId;

                readerBuffer.clear();
                this->deserializer = std::move(deserializer);
                return deserializerAccess.MakeShared(static_cast<MethodDeserializer&>(*this));
            }
            else
            {
                tracer.Trace() << "> Unknown service " << serviceId << " method " << methodId;
                return deserializer;
            }
        }

        void TracingEchoOnStreamsDescendantHelper::ReleaseDeserializer()
        {
            deserializer = nullptr;
        }

        bool TracingEchoOnStreamsDescendantHelper::Serialize(infra::SharedPtr<infra::StreamWriter>&& writer)
        {
            return serializer->Serialize(tracingWriter.Emplace(std::move(writer), *this));
        }

        void TracingEchoOnStreamsDescendantHelper::MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
        {
            auto start = reader->ConstructSaveMarker();
            while (!reader->Empty() && !readerBuffer.full())
            {
                auto range = reader->ExtractContiguousRange(readerBuffer.max_size() - readerBuffer.size());
                readerBuffer.insert(readerBuffer.end(), range.begin(), range.end());
            }
            reader->Rewind(start);

            deserializer->MethodContents(std::move(reader));
        }

        void TracingEchoOnStreamsDescendantHelper::ExecuteMethod()
        {
            infra::BoundedVectorInputStream stream(readerBuffer, infra::noFail);
            infra::ProtoLengthDelimited contentsCopy(stream, stream.ErrorPolicy(), static_cast<uint32_t>(stream.Available()));

            tracer.Trace() << "> ";
            receivingService->TraceMethod(receivingMethodId, contentsCopy, tracer);

            deserializer->ExecuteMethod();
        }

        bool TracingEchoOnStreamsDescendantHelper::Failed() const
        {
            return deserializer->Failed();
        }

        void TracingEchoOnStreamsDescendantHelper::SendingMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents) const
        {
            auto service = FindService(serviceId);

            if (service != nullptr)
            {
                tracer.Trace() << "< ";
                service->TraceMethod(methodId, contents, tracer);
            }
            else
                tracer.Trace() << "< Unknown service " << serviceId << " method " << methodId;
        }

        const ServiceTracer* TracingEchoOnStreamsDescendantHelper::FindService(uint32_t serviceId) const
        {
            for (auto& service : services)
                if (service.ServiceId() == serviceId)
                    return &service;

            return nullptr;
        }

        void TracingEchoOnStreamsDescendantHelper::SerializationDone()
        {
            while (!writerBuffer.empty())
            {
                infra::BoundedVectorInputStream stream(writerBuffer, infra::noFail);
                infra::StreamErrorPolicy formatErrorPolicy(infra::softFail);
                infra::ProtoParser parser(stream, formatErrorPolicy);
                auto serviceId = static_cast<uint32_t>(parser.GetVarInt());
                auto save = stream.Reader().ConstructSaveMarker();
                auto [contents, methodId] = parser.GetPartialField();

                if (stream.Failed() || (contents.Is<infra::PartialProtoLengthDelimited>() && contents.Get<infra::PartialProtoLengthDelimited>().length > stream.Available()))
                {
                    if (stream.Failed())
                        break;
                    else if (contents.Is<infra::PartialProtoLengthDelimited>() && writerBuffer.max_size() - writerBuffer.size() < contents.Get<infra::PartialProtoLengthDelimited>().length)
                    {
                        tracer.Trace() << "< message too big";
                        skipping = contents.Get<infra::PartialProtoLengthDelimited>().length;
                        writerBuffer.erase(writerBuffer.begin(), writerBuffer.begin() + stream.Reader().Processed());
                        auto skippingNow = std::min<std::size_t>(skipping, writerBuffer.size());
                        writerBuffer.erase(writerBuffer.begin(), writerBuffer.begin() + skippingNow);
                        skipping -= skippingNow;
                    }
                    else
                        break;
                }
                else
                {
                    stream.Reader().Rewind(save);
                    infra::ProtoParser newParser(stream, formatErrorPolicy);
                    auto message = newParser.GetField();

                    if (formatErrorPolicy.Failed() || !message.first.Is<infra::ProtoLengthDelimited>())
                    {
                        tracer.Trace() << "< Malformed message";
                        writerBuffer.erase(writerBuffer.begin(), writerBuffer.begin() + stream.Reader().Processed());
                    }
                    else
                    {
                        SendingMethod(serviceId, methodId, message.first.Get<infra::ProtoLengthDelimited>());
                        if (stream.Failed() || formatErrorPolicy.Failed())
                            tracer.Continue() << "... Malformed message";
                        writerBuffer.erase(writerBuffer.begin(), writerBuffer.begin() + stream.Reader().Processed());
                    }

                    serializer = nullptr;
                }
            }
        }

        TracingEchoOnStreamsDescendantHelper::TracingWriter::TracingWriter(infra::SharedPtr<infra::StreamWriter>&& delegate, TracingEchoOnStreamsDescendantHelper& echo)
            : delegate(std::move(delegate))
            , writer(echo.writerBuffer)
            , skipping(echo.skipping)
        {}

        void TracingEchoOnStreamsDescendantHelper::TracingWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
        {
            infra::StreamErrorPolicy tracingErrorPolicy(infra::noFail);
            writer.Insert(infra::DiscardHead(range, skipping), tracingErrorPolicy);
            skipping -= infra::Head(range, skipping).size();
            delegate->Insert(range, errorPolicy);
        }

        std::size_t TracingEchoOnStreamsDescendantHelper::TracingWriter::Available() const
        {
            return delegate->Available();
        }

        std::size_t TracingEchoOnStreamsDescendantHelper::TracingWriter::ConstructSaveMarker() const
        {
            return delegate->ConstructSaveMarker();
        }

        std::size_t TracingEchoOnStreamsDescendantHelper::TracingWriter::GetProcessedBytesSince(std::size_t marker) const
        {
            return delegate->GetProcessedBytesSince(marker);
        }

        infra::ByteRange TracingEchoOnStreamsDescendantHelper::TracingWriter::SaveState(std::size_t marker)
        {
            savedRange = writer.SaveState(writer.ConstructSaveMarker() - (delegate->ConstructSaveMarker() - marker));
            savedDelegateRange = delegate->SaveState(marker);
            return savedDelegateRange;
        }

        void TracingEchoOnStreamsDescendantHelper::TracingWriter::RestoreState(infra::ByteRange range)
        {
            auto restoreSize = savedDelegateRange.size() - range.size();
            auto from = infra::ByteRange(range.begin() - restoreSize, range.begin());

            infra::Copy(from, infra::Head(savedRange, restoreSize));
            writer.RestoreState(infra::DiscardHead(savedRange, restoreSize));

            savedRange = infra::ByteRange();
            delegate->RestoreState(range);
        }

        infra::ByteRange TracingEchoOnStreamsDescendantHelper::TracingWriter::Overwrite(std::size_t marker)
        {
            std::abort();
        }
    }
}
