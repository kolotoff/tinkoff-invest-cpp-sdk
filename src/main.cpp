#include <iostream>
#include <fstream>

#include "protos/users.grpc.pb.h"

#include <agrpc/asioGrpc.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

#include <Credentials.hpp>
#include <Request.hpp>

int main(int /*argc*/, const char** /*argv*/)
{
  using namespace tinkoff::Public::invest::api;

  const auto host = "invest-public-api.tinkoff.ru:443";

  Credentials credentials;
  if (!credentials.load())
  {
    return -1;
  }
  agrpc::GrpcContext grpc_context{ std::make_unique<grpc::CompletionQueue>() };

  Request<contract::v1::UsersService> testRequest(grpc_context, credentials);

  int errorCode = 0;

  grpc::SslCredentialsOptions ssl_options;
  ssl_options.pem_root_certs = credentials.certs();

  auto channel_credentials = grpc::SslCredentials(ssl_options);
  
  const auto stub = contract::v1::UsersService::NewStub(grpc::CreateChannel(host, channel_credentials));

  boost::asio::co_spawn(grpc_context, [&]() -> boost::asio::awaitable<void>
  {
    contract::v1::GetAccountsRequest request;
    auto result = co_await testRequest.execute(stub, &contract::v1::UsersService::Stub::AsyncGetAccounts, request);
    errorCode = result.status.error_code();
  },
  boost::asio::detached);

  grpc_context.run();

  return errorCode;
}