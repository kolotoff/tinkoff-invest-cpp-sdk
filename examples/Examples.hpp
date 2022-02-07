#include <iostream>
#include <fstream>

#include "protos/users.grpc.pb.h"
#include "protos/marketdata.grpc.pb.h"

#include <agrpc/asioGrpc.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

#include <Credentials.hpp>
#include <Service.hpp>
#include <StreamingService.hpp>
