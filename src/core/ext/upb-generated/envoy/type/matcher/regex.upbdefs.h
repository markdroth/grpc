/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/type/matcher/regex.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#ifndef ENVOY_TYPE_MATCHER_REGEX_PROTO_UPBDEFS_H_
#define ENVOY_TYPE_MATCHER_REGEX_PROTO_UPBDEFS_H_

#include "upb/def.h"
#include "upb/port_def.inc"
#ifdef __cplusplus
extern "C" {
#endif

#include "upb/def.h"

#include "upb/port_def.inc"

extern upb_def_init envoy_type_matcher_regex_proto_upbdefinit;

UPB_INLINE const upb_msgdef *envoy_type_matcher_RegexMatcher_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_type_matcher_regex_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.type.matcher.RegexMatcher");
}

UPB_INLINE const upb_msgdef *envoy_type_matcher_RegexMatcher_GoogleRE2_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_type_matcher_regex_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.type.matcher.RegexMatcher.GoogleRE2");
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#include "upb/port_undef.inc"

#endif  /* ENVOY_TYPE_MATCHER_REGEX_PROTO_UPBDEFS_H_ */
