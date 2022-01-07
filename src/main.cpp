#include "helper/helper.hpp"
#include "protos/users.grpc.pb.h"


#include <agrpc/asioGrpc.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

using namespace tinkoff::invest::api;

int main(int argc, const char** argv)
{
  const auto port = 443;
  const auto host = "invest-public-api.tinkoff.ru";

  grpc::Status status;

  static std::string token = "Bearer t.";

  grpc::SslCredentialsOptions ssl_options;
  ssl_options.pem_root_certs = SERVER_CRT;

  auto channel_credentials = grpc::SslCredentials(ssl_options);
  
  const auto stub = contract::v1::UsersService::NewStub(grpc::CreateChannel(host, channel_credentials));
  agrpc::GrpcContext grpc_context{std::make_unique<grpc::CompletionQueue>()};
  

  boost::asio::co_spawn(
    grpc_context,
    [&]() -> boost::asio::awaitable<void>
    {
      grpc::ClientContext client_context;
      client_context.AddMetadata("Authorization", token);
      contract::v1::GetAccountsRequest request;
      std::unique_ptr<grpc::ClientAsyncResponseReader<contract::v1::GetAccountsResponse>> reader =
        stub->AsyncGetAccounts(&client_context, request, agrpc::get_completion_queue(grpc_context));

      contract::v1::GetAccountsResponse response;
      co_await agrpc::finish(*reader, response, status);
    },
    boost::asio::detached);

  grpc_context.run();
  // end-snippet

  abort_if_not(status.ok());
}