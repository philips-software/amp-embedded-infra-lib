#ifndef HAL_USB_CUSTOM_HID_HPP
#define HAL_USB_CUSTOM_HID_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"

namespace hal
{
    class UsbCustomHid
    {
    public:
        UsbCustomHid() = default;
        UsbCustomHid(const UsbCustomHid&) = delete;
        UsbCustomHid& operator=(const UsbCustomHid&) = delete;

    protected:
        ~UsbCustomHid() = default;

    public:
        static const uint32_t ReportSize = 64;

        virtual void Send(infra::ConstByteRange data) = 0;
        virtual void OnReceived(infra::Function<void(infra::ConstByteRange)> onReportReceived) = 0;
    };
}

#endif
