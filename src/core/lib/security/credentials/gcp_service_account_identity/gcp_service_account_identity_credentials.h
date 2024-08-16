//
// Copyright 2024 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef GRPC_SRC_CORE_LIB_SECURITY_CREDENTIALS_GCP_SERVICE_ACCOUNT_IDENTITY_GCP_SERVICE_ACCOUNT_IDENTITY_CREDENTIALS_H
#define GRPC_SRC_CORE_LIB_SECURITY_CREDENTIALS_GCP_SERVICE_ACCOUNT_IDENTITY_GCP_SERVICE_ACCOUNT_IDENTITY_CREDENTIALS_H

#include <string>
#include <utility>

#include <grpc/credentials.h>
#include <grpc/grpc_security.h>

#include "src/core/lib/gprpp/orphanable.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/gprpp/unique_type_name.h"
#include "src/core/lib/iomgr/closure.h"
#include "src/core/lib/iomgr/polling_entity.h"
#include "src/core/lib/security/credentials/credentials.h"
#include "src/core/lib/security/credentials/token_fetcher/token_fetcher_credentials.h"
#include "src/core/lib/slice/slice.h"
#include "src/core/lib/transport/metadata.h"
#include "src/core/util/http_client/httpcli.h"
#include "src/core/util/http_client/parser.h"

namespace grpc_core {

// A JWT token.
class JwtToken : public TokenFetcherCredentials::Token {
 public:
  JwtToken(Slice token, Timestamp expiration)
      : token_(std::move(token)), expiration_(expiration) {}

  Timestamp ExpirationTime() override { return expiration_; }

  void AddTokenToClientInitialMetadata(ClientMetadata& metadata) override {
    metadata.Append(GRPC_AUTHORIZATION_METADATA_KEY, token_.Ref(),
                    [](absl::string_view, const Slice&) { abort(); });
  }

 private:
  Slice token_;
  Timestamp expiration_;
};

// A base class for JWT token fetching credentials.
// Subclasses must implement StartHttpRequest().
class JwtTokenFetcherCallCredentials : public TokenFetcherCredentials {
 public:
  OrphanablePtr<FetchRequest> FetchToken(
      Timestamp deadline,
      absl::AnyInvocable<
          void(absl::StatusOr<RefCountedPtr<TokenFetcherCredentials::Token>>)>
          on_done) final;

 private:
  class HttpFetchRequest;

  virtual OrphanablePtr<HttpRequest> StartHttpRequest(
      grpc_polling_entity* pollent, Timestamp deadline,
      grpc_http_response* response, grpc_closure* on_complete) = 0;
};

// GCP service account identity call credentials.
class GcpServiceAccountIdentityCallCredentials
    : public JwtTokenFetcherCallCredentials {
 public:
  explicit GcpServiceAccountIdentityCallCredentials(std::string audience)
      : audience_(std::move(audience)) {}

  std::string debug_string() override;

  UniqueTypeName type() const override;

  absl::string_view audience() const { return audience_; }

 private:
  OrphanablePtr<HttpRequest> StartHttpRequest(
      grpc_polling_entity* pollent, Timestamp deadline,
      grpc_http_response* response, grpc_closure* on_complete) override;

  int cmp_impl(const grpc_call_credentials* other) const override {
    // TODO(yashykt): Check if we can do something better here
    return QsortCompare(static_cast<const grpc_call_credentials*>(this), other);
  }

  std::string audience_;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_SECURITY_CREDENTIALS_GCP_SERVICE_ACCOUNT_IDENTITY_GCP_SERVICE_ACCOUNT_IDENTITY_CREDENTIALS_H
