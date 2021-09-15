#ifndef HAL_USB_LINK_LAYER_HPP
#define HAL_USB_LINK_LAYER_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Observer.hpp"
#include <cstdint>

namespace hal
{
    class UsbLinkLayer;

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

    class UsbLinkLayerObserver
        : public infra::SingleObserver<UsbLinkLayerObserver, UsbLinkLayer>
    {
    protected:
        UsbLinkLayerObserver(UsbLinkLayer& linkLayer)
            : infra::SingleObserver<UsbLinkLayerObserver, UsbLinkLayer>(linkLayer)
        {}
        ~UsbLinkLayerObserver() = default;

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

    class UsbDeviceFactory
    {
    public:
        UsbDeviceFactory() = default;
        UsbDeviceFactory(const UsbDeviceFactory& other) = delete;
        UsbDeviceFactory& operator=(const UsbDeviceFactory& other) = delete;

        virtual void Create(UsbLinkLayer& linkLayer) = 0;
        virtual void Destroy() = 0;

    protected:
        ~UsbDeviceFactory() = default;
    };

    class UsbLinkLayer
        : public infra::Subject<UsbLinkLayerObserver>
    {
    public:
        UsbLinkLayer() = default;
        UsbLinkLayer(const UsbLinkLayer& other) = delete;
        UsbLinkLayer& operator=(const UsbLinkLayer& other) = delete;

    protected:
        ~UsbLinkLayer() = default;

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
}

#endif
