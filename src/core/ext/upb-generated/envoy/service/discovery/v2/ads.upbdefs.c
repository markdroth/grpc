/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/service/discovery/v2/ads.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init envoy_api_v2_discovery_proto_upbdefinit;
extern upb_def_init udpa_annotations_status_proto_upbdefinit;
extern const upb_msglayout envoy_service_discovery_v2_AdsDummy_msginit;

static const upb_msglayout *layouts[1] = {
  &envoy_service_discovery_v2_AdsDummy_msginit,
};

static const char descriptor[454] =
  "\n$envoy/service/discovery/v2/ads.proto\022\032"
  "envoy.service.discovery.v2\032\034envoy/api/v2"
  "/discovery.proto\032\035udpa/annotations/statu"
  "s.proto\"\n\n\010AdsDummy2\355\001\n\032AggregatedDiscov"
  "eryService\022b\n\031StreamAggregatedResources\022"
  "\036.envoy.api.v2.DiscoveryRequest\032\037.envoy."
  "api.v2.DiscoveryResponse\"\000(\0010\001\022k\n\030DeltaA"
  "ggregatedResources\022#.envoy.api.v2.DeltaD"
  "iscoveryRequest\032$.envoy.api.v2.DeltaDisc"
  "overyResponse\"\000(\0010\001BA\n(io.envoyproxy.env"
  "oy.service.discovery.v2B\010AdsProtoP\001\210\001\001\272\200"
  "\310\321\006\002\020\001b\006proto3";

static upb_def_init *deps[3] = {
  &envoy_api_v2_discovery_proto_upbdefinit,
  &udpa_annotations_status_proto_upbdefinit,
  NULL
};

upb_def_init envoy_service_discovery_v2_ads_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/service/discovery/v2/ads.proto",
  UPB_STRVIEW_INIT(descriptor, 454)
};
