/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/api/v2/endpoint/endpoint.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#ifndef ENVOY_API_V2_ENDPOINT_ENDPOINT_PROTO_UPBDEFS_H_
#define ENVOY_API_V2_ENDPOINT_ENDPOINT_PROTO_UPBDEFS_H_

#include "upb/def.h"
#include "upb/port_def.inc"
#ifdef __cplusplus
extern "C" {
#endif

#include "upb/def.h"

#include "upb/port_def.inc"

extern upb_def_init envoy_api_v2_endpoint_endpoint_proto_upbdefinit;

UPB_INLINE const upb_msgdef *envoy_api_v2_endpoint_Endpoint_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_api_v2_endpoint_endpoint_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.api.v2.endpoint.Endpoint");
}

UPB_INLINE const upb_msgdef *envoy_api_v2_endpoint_Endpoint_HealthCheckConfig_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_api_v2_endpoint_endpoint_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.api.v2.endpoint.Endpoint.HealthCheckConfig");
}

UPB_INLINE const upb_msgdef *envoy_api_v2_endpoint_LbEndpoint_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_api_v2_endpoint_endpoint_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.api.v2.endpoint.LbEndpoint");
}

UPB_INLINE const upb_msgdef *envoy_api_v2_endpoint_LocalityLbEndpoints_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_api_v2_endpoint_endpoint_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.api.v2.endpoint.LocalityLbEndpoints");
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#include "upb/port_undef.inc"

#endif  /* ENVOY_API_V2_ENDPOINT_ENDPOINT_PROTO_UPBDEFS_H_ */
