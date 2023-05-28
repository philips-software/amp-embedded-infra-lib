#ifndef HAL_USB_LINK_LAYER_HPP
#define HAL_USB_LINK_LAYER_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Observer.hpp"
#include <cstdint>

namespace hal
{
    enum class UsbSpeed : uint8_t
    {
        high = 0,
        full = 1,
        low = 2
    };

    enum class UsbEndPointType : uint8_t
    {
        control = 0,
        isochronous = 1,
        bulk = 2,
        interrupt = 3
    };

    enum struct UsbUrbState : uint8_t
    {
        idle,
        done,
        notReady,
        notYet,
        error,
        stall,
        invalid,
    };

    enum struct UsbPid : uint8_t
    {
        setup,
        data,
        invalid,
    };

    struct UsbPipe
    {
        uint8_t end_point;
        uint8_t address;
        UsbSpeed speed;
        UsbEndPointType type;
        uint16_t max_packet_size;
    };

    struct UsbUrb
    {
        bool direction_is_in;
        UsbEndPointType type;
        UsbPid token;
        infra::ByteRange buffer;
        bool do_ping;
    };

    class UsbDeviceLinkLayer;
    class UsbHostLinkLayer;

    template <class T>
    class UsbDeviceFactory
    {
    public:
        UsbDeviceFactory() = default;
        UsbDeviceFactory(const UsbDeviceFactory& other) = delete;
        UsbDeviceFactory& operator=(const UsbDeviceFactory& other) = delete;

        virtual void Create(T& linkLayer) = 0;
        virtual void Destroy() = 0;

    protected:
        ~UsbDeviceFactory() = default;
    };

    class UsbDeviceLinkLayerObserver
        : public infra::SingleObserver<UsbDeviceLinkLayerObserver, UsbDeviceLinkLayer>
    {
    protected:
        UsbDeviceLinkLayerObserver(UsbDeviceLinkLayer& linkLayer)
            : infra::SingleObserver<UsbDeviceLinkLayerObserver, UsbDeviceLinkLayer>(linkLayer)
        {}

        ~UsbDeviceLinkLayerObserver() = default;

    public:
        virtual void SetupStage(infra::ConstByteRange setup) = 0;
        virtual void DataOutStage(uint8_t epnum, infra::ConstByteRange data) = 0;
        virtual void DataInStage(uint8_t epnum, infra::ConstByteRange data) = 0;

        virtual void Suspend() = 0;
        virtual void Resume() = 0;

        virtual void StartOfFrame() = 0;
        virtual void IsochronousInIncomplete(uint8_t epnum) = 0;
        virtual void IsochronousOutIncomplete(uint8_t epnum) = 0;
    };

    class UsbDeviceLinkLayer
        : public infra::Subject<UsbDeviceLinkLayerObserver>
    {
    public:
        UsbDeviceLinkLayer() = default;
        UsbDeviceLinkLayer(const UsbDeviceLinkLayer& other) = delete;
        UsbDeviceLinkLayer& operator=(const UsbDeviceLinkLayer& other) = delete;

    protected:
        ~UsbDeviceLinkLayer() = default;

    public:
        virtual void OpenEndPoint(uint8_t ep_addr, UsbEndPointType type, uint16_t ep_mps) = 0;
        virtual void CloseEndPoint(uint8_t ep_addr) = 0;
        virtual void FlushEndPoint(uint8_t ep_addr) = 0;
        virtual void StallEndPoint(uint8_t ep_addr) = 0;
        virtual void ClearStallEndPoint(uint8_t ep_addr) = 0;
        virtual bool IsStallEndPoint(uint8_t ep_addr) = 0;
        virtual void SetUsbAddress(uint8_t dev_addr) = 0;
        virtual void Transmit(uint8_t ep_addr, infra::ConstByteRange data) = 0;
        virtual void PrepareReceive(uint8_t ep_addr, infra::ConstByteRange data) = 0;
        virtual uint32_t GetReceiveDataSize(uint8_t ep_addr) = 0;
    };

    class UsbHostLinkLayerObserver
        : public infra::SingleObserver<UsbHostLinkLayerObserver, UsbHostLinkLayer>
    {
    protected:
        UsbHostLinkLayerObserver(UsbHostLinkLayer& linkLayer)
            : infra::SingleObserver<UsbHostLinkLayerObserver, UsbHostLinkLayer>(linkLayer)
        {}

        ~UsbHostLinkLayerObserver() = default;

    public:
        virtual void Connected() = 0;
        virtual void Disconnected() = 0;
        virtual void PortEnabled() = 0;
        virtual void PortDisabled() = 0;
        virtual void StartOfFrame() = 0;
    };

    class UsbHostLinkLayer
        : public infra::Subject<UsbHostLinkLayer>
    {
    public:
        UsbHostLinkLayer() = default;
        UsbHostLinkLayer(const UsbHostLinkLayer& other) = delete;
        UsbHostLinkLayer& operator=(const UsbHostLinkLayer& other) = delete;

    protected:
        ~UsbHostLinkLayer() = default;

    public:
        virtual UsbSpeed Speed() = 0;
        virtual void ResetPort() = 0;

        virtual void OpenPipe(uint8_t pipe, UsbPipe context) = 0;
        virtual void ClosePipe(uint8_t pipe) = 0;
        virtual void SubmitUrb(uint8_t pipe, UsbUrb urb) = 0;
        virtual UsbUrbState GetUrbState(uint8_t pipe) = 0;
        virtual uint32_t GetLastTransferSize(uint8_t pipe) = 0;

        virtual uint32_t GetCurrentFrame() = 0;

        virtual void SetToggle(uint8_t pipe, bool toggle) = 0;
        virtual bool GetToggle(uint8_t pipe) = 0;
    };
}

#endif
