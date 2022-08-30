// Copyright 2022 gRPC authors.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/core/lib/promise/promise.h"
#include "src/core/lib/resource_quota/resource_quota.h"
#include "src/core/lib/security/context/security_context.h"
#include "src/core/lib/security/credentials/fake/fake_credentials.h"
#include "src/core/lib/security/security_connector/fake/fake_security_connector.h"
#include "src/core/lib/security/transport/auth_filters.h"
#include "test/core/promise/test_context.h"

namespace grpc_core {
namespace {

// FIXME: move into ClientAuthFilterTest?
auto* g_memory_allocator = new MemoryAllocator(
    ResourceQuota::Default()->memory_quota()->CreateMemoryAllocator("test"));

class ClientAuthFilterTest : public ::testing::Test {
 protected:
  class FailCallCreds : public grpc_call_credentials {
   public:
    explicit FailCallCreds(absl::Status status)
        : grpc_call_credentials(GRPC_SECURITY_NONE),
          status_(std::move(status)) {}

    UniqueTypeName type() const override {
      static UniqueTypeName::Factory kFactory("FailCallCreds");
      return kFactory.Create();
    }

    ArenaPromise<absl::StatusOr<ClientMetadataHandle>> GetRequestMetadata(
        ClientMetadataHandle initial_metadata,
        const GetRequestMetadataArgs* args) override {
      return Immediate<absl::StatusOr<ClientMetadataHandle>>(status_);
    }

    int cmp_impl(const grpc_call_credentials* other) const override {
      return QsortCompare(
          status_.ToString(),
          static_cast<const FailCallCreds*>(other)->status_.ToString());
    }

   private:
    absl::Status status_;
  };

  void SetUp() override {
    channel_creds_.reset(grpc_fake_transport_security_credentials_create());
  }

  ChannelArgs MakeChannelArgs(absl::Status status_for_call_creds) {
    ChannelArgs args;
    auto security_connector = channel_creds_->create_security_connector(
        status_for_call_creds.ok()
            ? nullptr
            : MakeRefCounted<FailCallCreds>(std::move(status_for_call_creds)),
        "localhost:1234", &args);
    return args.SetObject(std::move(security_connector))
        .SetObject(MakeRefCounted<grpc_auth_context>(nullptr));
  }

  RefCountedPtr<grpc_channel_credentials> channel_creds_;
};

TEST_F(ClientAuthFilterTest, CreateFailsWithoutRequiredChannelArgs) {
  EXPECT_FALSE(
      ClientAuthFilter::Create(ChannelArgs(), ChannelFilter::Args()).ok());
}

TEST_F(ClientAuthFilterTest, CreateSucceeds) {
  auto filter = ClientAuthFilter::Create(MakeChannelArgs(absl::OkStatus()),
                                         ChannelFilter::Args());
  EXPECT_TRUE(filter.ok()) << filter.status();
}

TEST_F(ClientAuthFilterTest, CallCredsFails) {
  auto filter = ClientAuthFilter::Create(
      MakeChannelArgs(absl::UnauthenticatedError("access denied")),
      ChannelFilter::Args());
  auto arena = MakeScopedArena(1024, g_memory_allocator);
  grpc_metadata_batch initial_metadata_batch(arena.get());
  grpc_metadata_batch trailing_metadata_batch(arena.get());
  // TODO(ctiller): use Activity here, once it's ready.
  TestContext<Arena> context(arena.get());
  grpc_call_context_element call_context[GRPC_CONTEXT_COUNT];
  TestContext<grpc_call_context_element> promise_call_context(call_context);
  auto promise = filter->MakeCallPromise(
      CallArgs{
          ClientMetadataHandle::TestOnlyWrap(&initial_metadata_batch),
          nullptr,
      },
      [&](CallArgs call_args) {
        return ArenaPromise<ServerMetadataHandle>(
            [&]() -> Poll<ServerMetadataHandle> {
              return ServerMetadataHandle::TestOnlyWrap(
                  &trailing_metadata_batch);
            });
      });
  auto result = promise();
  ServerMetadataHandle* server_metadata =
      absl::get_if<ServerMetadataHandle>(&result);
  ASSERT_TRUE(server_metadata != nullptr);
  auto status_md = (*server_metadata)->get(GrpcStatusMetadata());
  ASSERT_TRUE(status_md.has_value());
  EXPECT_EQ(*status_md, GRPC_STATUS_UNAUTHENTICATED);
  const Slice* message_md =
      (*server_metadata)->get_pointer(GrpcMessageMetadata());
  ASSERT_TRUE(message_md != nullptr);
  EXPECT_EQ(message_md->as_string_view(), "access denied");
}

TEST_F(ClientAuthFilterTest, RewritesInvalidStatusFromCallCreds) {
  auto filter = ClientAuthFilter::Create(
      MakeChannelArgs(absl::AbortedError("nope")), ChannelFilter::Args());
  auto arena = MakeScopedArena(1024, g_memory_allocator);
  grpc_metadata_batch initial_metadata_batch(arena.get());
  grpc_metadata_batch trailing_metadata_batch(arena.get());
  // TODO(ctiller): use Activity here, once it's ready.
  TestContext<Arena> context(arena.get());
  grpc_call_context_element call_context[GRPC_CONTEXT_COUNT];
  TestContext<grpc_call_context_element> promise_call_context(call_context);
  auto promise = filter->MakeCallPromise(
      CallArgs{
          ClientMetadataHandle::TestOnlyWrap(&initial_metadata_batch),
          nullptr,
      },
      [&](CallArgs call_args) {
        return ArenaPromise<ServerMetadataHandle>(
            [&]() -> Poll<ServerMetadataHandle> {
              return ServerMetadataHandle::TestOnlyWrap(
                  &trailing_metadata_batch);
            });
      });
  auto result = promise();
  ServerMetadataHandle* server_metadata =
      absl::get_if<ServerMetadataHandle>(&result);
  ASSERT_TRUE(server_metadata != nullptr);
  auto status_md = (*server_metadata)->get(GrpcStatusMetadata());
  ASSERT_TRUE(status_md.has_value());
  EXPECT_EQ(*status_md, GRPC_STATUS_INTERNAL);
  const Slice* message_md =
      (*server_metadata)->get_pointer(GrpcMessageMetadata());
  ASSERT_TRUE(message_md != nullptr);
  EXPECT_EQ(message_md->as_string_view(),
            "illegal status code from call credentials; original status: "
            "ABORTED: nope");
}

}  // namespace
}  // namespace grpc_core

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  grpc_init();
  int retval = RUN_ALL_TESTS();
  grpc_shutdown();
  return retval;
}
