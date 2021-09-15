/*
Generated by LwipMibCompiler
*/

#ifndef LWIP_HDR_APPS_SNMP_FRAMEWORK_MIB_H
#define LWIP_HDR_APPS_SNMP_FRAMEWORK_MIB_H

#include "lwip/apps/snmp_opts.h"
#if LWIP_SNMP

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "lwip/apps/snmp_core.h"

    extern const struct snmp_obj_id usmNoAuthProtocol;
    extern const struct snmp_obj_id usmHMACMD5AuthProtocol;
    extern const struct snmp_obj_id usmHMACSHAAuthProtocol;

    extern const struct snmp_obj_id usmNoPrivProtocol;
    extern const struct snmp_obj_id usmDESPrivProtocol;
    extern const struct snmp_obj_id usmAESPrivProtocol;

    extern const struct snmp_mib snmpframeworkmib;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWIP_SNMP */
#endif /* LWIP_HDR_APPS_SNMP_FRAMEWORK_MIB_H */
