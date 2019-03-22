#ifndef LIGHTWEIGHT_IP_CC_H
#define LIGHTWEIGHT_IP_CC_H

#include <stdlib.h>
#include <stdint.h>

#define U16_F "hu"
#define S16_F "d"
#define X16_F "hx"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"
#define SZT_F "uz"

#if defined (__GNUC__)

#define BYTE_ORDER LITTLE_ENDIAN

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#elif defined (_MSC_VER)

#define BYTE_ORDER LITTLE_ENDIAN

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#else

#error Unsupported compiler

#endif

#define LWIP_PLATFORM_ASSERT(x) do { abort(); } while (0)

#endif
