#ifndef BLE_CORE_SPECIFICATION_DEFINITIONS_HPP
#define BLE_CORE_SPECIFICATION_DEFINITIONS_HPP

#include <cstdint>

namespace services
{
    class Ble
    {
    public:
        //Core Specification Supplement, Part A, Section 1.3
        enum class ADType : uint8_t
        {
            FLAGS                                = 0x01U,
            INCMPLT_LIST_16_BIT_SERV_UUID        = 0x02U,
            CMPLT_LIST_16_BIT_SERV_UUID          = 0x03U,
            INCMPLT_LIST_32_BIT_SERV_UUID        = 0x04U,
            CMPLT_LIST_32_BIT_SERV_UUID          = 0x05U,
            INCMPLT_LIST_128_BIT_SERV_UUID       = 0x06U,
            CMPLT_LIST_128_BIT_SERV_UUID         = 0x07U,
            SHORTENED_LOCAL_NAME                 = 0x08U,
            COMPLETE_LOCAL_NAME                  = 0x09U,
            TX_POWER_LEVEL                       = 0x0AU,
            CLASS_OF_DEVICE                      = 0x0DU,
            SEC_MGR_TK_VALUE                     = 0x10U,
            SEC_MGR_OOB_FLAGS                    = 0x11U,
            SLAVE_CONN_INTERVAL                  = 0x12U,
            SERV_SOLICIT_16_BIT_UUID_LIST        = 0x14U,
            SERV_SOLICIT_128_BIT_UUID_LIST       = 0x15U,
            SERVICE_DATA                         = 0x16U,
            APPEARANCE                           = 0x19U,
            ADVERTISING_INTERVAL                 = 0x1AU,
            LE_ROLE                              = 0x1CU,
            SERV_SOLICIT_32_BIT_UUID_LIST        = 0x1FU,
            URI                                  = 0x24U,
            MANUFACTURER_SPECIFIC_DATA           = 0xFFU
        };

        //Core Specification Supplement, Part A, Section 1.3
        typedef ADType SRDType;

        static constexpr uint8_t maxAdvertisementSize = 31;
        static constexpr uint8_t maxScanResponseSize = 31;
    };
}

#endif
