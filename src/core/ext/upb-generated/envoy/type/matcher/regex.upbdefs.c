/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/type/matcher/regex.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init google_protobuf_wrappers_proto_upbdefinit;
extern upb_def_init validate_validate_proto_upbdefinit;
extern const upb_msglayout envoy_type_matcher_RegexMatcher_msginit;
extern const upb_msglayout envoy_type_matcher_RegexMatcher_GoogleRE2_msginit;

static const upb_msglayout *layouts[2] = {
  &envoy_type_matcher_RegexMatcher_msginit,
  &envoy_type_matcher_RegexMatcher_GoogleRE2_msginit,
};

static const char descriptor[413] =
  "\n\036envoy/type/matcher/regex.proto\022\022envoy."
  "type.matcher\032\036google/protobuf/wrappers.p"
  "roto\032\027validate/validate.proto\"\363\001\n\014RegexM"
  "atcher\022W\n\ngoogle_re2\030\001 \001(\0132*.envoy.type."
  "matcher.RegexMatcher.GoogleRE2B\n\272\351\300\003\005\212\001\002"
  "\020\001H\000R\tgoogleRe2\022\037\n\005regex\030\002 \001(\tB\t\272\351\300\003\004r\002 "
  "\001R\005regex\032S\n\tGoogleRE2\022F\n\020max_program_siz"
  "e\030\001 \001(\0132\034.google.protobuf.UInt32ValueR\016m"
  "axProgramSizeB\024\n\013engine_type\022\005\270\351\300\003\001B0\n i"
  "o.envoyproxy.envoy.type.matcherB\nRegexPr"
  "otoP\001b\006proto3";

static upb_def_init *deps[3] = {
  &google_protobuf_wrappers_proto_upbdefinit,
  &validate_validate_proto_upbdefinit,
  NULL
};

upb_def_init envoy_type_matcher_regex_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/type/matcher/regex.proto",
  UPB_STRVIEW_INIT(descriptor, 413)
};
