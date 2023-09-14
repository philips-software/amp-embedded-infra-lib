#ifndef PROTOBUF_PROTO_MESSAGE_BUILDER_HPP
#define PROTOBUF_PROTO_MESSAGE_BUILDER_HPP

#include "infra/stream/ByteInputStream.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "infra/util/BoundedDeque.hpp"

namespace services
{
    class BufferingStreamReader
        : public infra::StreamReader
    {
    public:
        BufferingStreamReader(infra::BoundedDeque<uint8_t>& buffer, infra::ConstByteRange data);

        void ConsumeCurrent();
        void StoreRemainder();

        // Implementation of StreamReader
        void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
        infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
        infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;

    private:
        infra::BoundedDeque<uint8_t>& buffer;
        infra::ConstByteRange data;
        std::size_t index = 0;
    };

    template<class Message>
    class ProtoMessageBuilder
    {
    public:
        void Feed(infra::ConstByteRange data);

    public:
        Message message;

    private:
        infra::BoundedDeque<uint8_t>::WithMaxSize<16> buffer;
    };
}

//// Implementation ////

namespace services
{
    template<class Message>
    void ProtoMessageBuilder<Message>::Feed(infra::ConstByteRange data)
    {
        BufferingStreamReader reader{ buffer, data };
        infra::DataInputStream::WithErrorPolicy stream{ reader, infra::softFail };
        infra::ProtoParser parser{ stream };

        while (true)
        {
            auto field = parser.GetField();
            switch (field.second)
            {
                case Message::template fieldNumber<0>:
                    DeserializeField(Message::template ProtoType<0>(), parser, field, message.value);
                    break;
                // default:
                //     if (field.first.Is<infra::ProtoLengthDelimited>())
                //         field.first.Get<infra::ProtoLengthDelimited>().SkipEverything();
                //     break;
            }

            if (stream.Failed())
                break;

            reader.ConsumeCurrent();
        }

        reader.StoreRemainder();
    }
}

#endif
