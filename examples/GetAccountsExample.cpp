#include "Examples.hpp"

int main(int /*argc*/, const char** /*argv*/)
{
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif

  using namespace tinkoff::Public::invest::api;

  Credentials credentials;
  if (!credentials.load())
  {
    return -1;
  }
  agrpc::GrpcContext grpcExecutionContext { std::make_unique<grpc::CompletionQueue>() };

  int errorCode = 0;

  using UsersService = Service<contract::v1::UsersService>;
  UsersService usersService(credentials);

  boost::asio::co_spawn(grpcExecutionContext, [&]() -> boost::asio::awaitable<void>
  {
    auto result = co_await usersService.execute(&UsersService::Method::AsyncGetAccounts, {});
    errorCode = result.status.error_code();
  },
  boost::asio::detached);

  grpcExecutionContext.run();

  return errorCode;
}