#include <iostream>
#include <fstream>

#include "protos/users.grpc.pb.h"

#include <agrpc/asioGrpc.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>


bool readFile(const std::string& filePath, std::string& buffer)
{
  std::ifstream srcFile(filePath, std::ios::binary);
  if (!srcFile.is_open())
  {
    return false;
  }

  std::string tmp((std::istreambuf_iterator<char>(srcFile)), std::istreambuf_iterator<char>());
  if (tmp.size() < 10)
  {
    return false;
  }

  buffer = tmp;

  return true;
}

int main(int /*argc*/, const char** /*argv*/)
{
  using namespace tinkoff::Public::invest::api;

  const std::string rootCertFilePath = "roots.pem";
  const std::string tokenFilePath = "token.txt";

  std::string pemRootCerts;
  if (!readFile(rootCertFilePath, pemRootCerts))
  {
    return -1;
  }

  std::string token;
  if (!readFile(tokenFilePath, token))
  {
    return -2;
  }

  const auto host = "invest-public-api.tinkoff.ru:443";

  grpc::Status status;

  grpc::SslCredentialsOptions ssl_options;
  ssl_options.pem_root_certs = pemRootCerts;

  auto channel_credentials = grpc::SslCredentials(ssl_options);
  
  const auto stub = contract::v1::UsersService::NewStub(grpc::CreateChannel(host, channel_credentials));
  agrpc::GrpcContext grpc_context{std::make_unique<grpc::CompletionQueue>()};
  contract::v1::GetAccountsResponse response;
  grpc::ClientContext context;

  boost::asio::co_spawn(grpc_context, [&]() -> boost::asio::awaitable<void>
  {
    context.AddMetadata("authorization", "Bearer " + token);
    contract::v1::GetAccountsRequest request;
    std::unique_ptr<grpc::ClientAsyncResponseReader<contract::v1::GetAccountsResponse>> reader =
      stub->AsyncGetAccounts(&context, request, agrpc::get_completion_queue(grpc_context));

      
    co_await agrpc::finish(*reader, response, status);
  },
  boost::asio::detached);

  grpc_context.run();

  std::cout << std::endl << status.error_message() << " " << status.error_details();

  auto printMetadata = [](const auto& metadata)
  {
    for (auto iter = metadata.begin(); iter != metadata.end(); ++iter)
    {
      std::cout << iter->first << ": ";
      const size_t isBinary = iter->first.find("-bin");
      if ((isBinary != std::string::npos) && (isBinary + 4 == iter->first.size())) 
      {
        std::cout << std::hex;
        for (auto c : iter->second) 
        {
          std::cout << static_cast<unsigned int>(c);
        }
        std::cout << std::dec;
      }
      else 
      {
        std::cout << iter->second;
      }
      std::cout << std::endl;
    }
  };

  if (status.ok())
  {
    std::cout << "initial metadata: " << std::endl;
    printMetadata(context.GetServerInitialMetadata());

    std::cout << "trailing metadata: " << std::endl;
    printMetadata(context.GetServerTrailingMetadata());
  }

  getchar();

  return status.error_code();
}