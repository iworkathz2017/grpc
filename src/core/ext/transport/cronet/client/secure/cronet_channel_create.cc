/*
 *
 * Copyright 2016 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <grpc/support/port_platform.h>

#include "src/core/ext/transport/cronet/client/secure/cronet_channel_create.h"

#include <stdio.h>
#include <string.h>

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>

#include "src/core/ext/transport/cronet/transport/cronet_transport.h"
#include "src/core/lib/resource_quota/api.h"
#include "src/core/lib/surface/channel.h"
#include "src/core/lib/transport/transport_impl.h"

// Cronet transport object
typedef struct cronet_transport {
  grpc_transport base;  // must be first element in this structure
  void* engine;
  char* host;
} cronet_transport;

extern grpc_transport_vtable grpc_cronet_vtable;

GRPCAPI grpc_channel* grpc_cronet_secure_channel_create(
    void* engine, const char* target, const grpc_channel_args* args,
    void* reserved) {
  gpr_log(GPR_DEBUG,
          "grpc_create_cronet_transport: stream_engine = %p, target=%s", engine,
          target);

  // Disable client authority filter when using Cronet
  args = grpc_core::CoreConfiguration::Get()
             .channel_args_preconditioning()
             .PreconditionChannelArgs(args)
             .Set(GRPC_ARG_DISABLE_CLIENT_AUTHORITY_FILTER, 1)
             .ToC();

  grpc_transport* ct =
      grpc_create_cronet_transport(engine, target, args, reserved);

  grpc_core::ExecCtx exec_ctx;
  auto channel =
      grpc_core::Channel::Create(target, grpc_core::ChannelArgs::FromC(args),
                                 GRPC_CLIENT_DIRECT_CHANNEL, ct);
  grpc_channel_args_destroy(args);
  return channel.ok() ? channel->release()->c_ptr() : nullptr;
}
