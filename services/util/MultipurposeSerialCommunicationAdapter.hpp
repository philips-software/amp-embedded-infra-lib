#ifndef SERVICES_UTIL_MULTIPURPOSE_SERIAL_COMMUNICATION_ADAPTER_HPP
#define SERVICES_UTIL_MULTIPURPOSE_SERIAL_COMMUNICATION_ADAPTER_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "hal/synchronous_interfaces/SynchronousSerialCommunication.hpp"
#include <type_traits>
#include <utility>
#include <variant>

namespace hal
{
    template<class Synchronous, class Asynchronous,
        typename = std::enable_if_t<std::is_base_of_v<SynchronousSerialCommunication, Synchronous> && std::is_base_of_v<SerialCommunication, Asynchronous>>>
    class MultpurposeSerialCommuniationAdapter
        : public SerialCommunication
        , public SynchronousSerialCommunication
    {
    public:
        // Implementation of SerialCommunication
        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override
        {
            if (isSynchronous)
                SwapAsync();

            std::get<Asynchronous>(serial).SendData(data, actionOnCompletion);
        }

        void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override
        {
            if (isSynchronous)
                SwapAsync();

            std::get<Asynchronous>(serial).ReceiveData(dataReceived);
        }

        // Implementation of SynchronousSerialCommunication
        void SendData(infra::ConstByteRange data) override
        {
            if (!isSynchronous)
                SwapSync();

            std::get<Synchronous>(serial).SendData(data);
        }

        bool ReceiveData(infra::ByteRange data) override
        {
            if (!isSynchronous)
                SwapSync();

            return std::get<Synchronous>(serial).ReceiveData(data);
        }

    protected:
        template<class... Args>
        explicit MultpurposeSerialCommuniationAdapter(std::in_place_type_t<Synchronous>, Args&&... args)
            : serial(std::in_place_type<Synchronous>, std::forward<Args>(args)...)
        {
        }

        bool IsSynchronous() const
        {
            return isSynchronous;
        }

        template<class Type, class... Args>
        void Emplace(Args&&... args)
        {
            serial.template emplace<Type>(std::forward<Args>(args)...);
        }

        template<class Type>
        Type& Get()
        {
            return std::get<Type>(serial);
        }

    private:
        void SwapAsync()
        {
            isSynchronous = false;
            EmplaceAsync();
        }

        void SwapSync()
        {
            isSynchronous = true;
            EmplaceSync();
        }

        virtual void EmplaceSync() = 0;
        virtual void EmplaceAsync() = 0;

    private:
        std::variant<Synchronous, Asynchronous> serial;
        bool isSynchronous = true;
    };
}

#endif
