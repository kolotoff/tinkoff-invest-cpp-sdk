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

  using MarketDataStreamService = StreamingService<contract::v1::MarketDataStreamService>;
  MarketDataStreamService marketDataStreamService(credentials);

  boost::asio::co_spawn(grpcExecutionContext, [&]() -> boost::asio::awaitable<void>
  {
    {
      auto result = co_await usersService.execute(&UsersService::Method::AsyncGetAccounts, {});
      errorCode = result.status.error_code();
    }
    
    {
      auto method = &UsersService::Method::AsyncGetUserTariff;
      auto request = usersService.makeRequest(method);
      auto result = co_await usersService.execute(method, request);
      errorCode = result.status.error_code();
    }

    {
      static const std::string appleFigi = "BBG000B9XRY4"; //Apple
      static const std::string mvideoFigi = "BBG004S68CP5"; //MVID
      static const std::string spyf3_22 = "FUTSPYF03220"; //SPYF-3.22

      auto method = &MarketDataStreamService::Method::AsyncMarketDataStream;
      auto request = marketDataStreamService.makeRequest(method);

      auto candlesRequest = request.mutable_subscribe_candles_request();
      candlesRequest->set_subscription_action(contract::v1::SubscriptionAction::SUBSCRIPTION_ACTION_SUBSCRIBE);
      
      {
        auto instrument = candlesRequest->add_instruments();
        instrument->set_figi(spyf3_22);
        instrument->set_interval(contract::v1::SubscriptionInterval::SUBSCRIPTION_INTERVAL_ONE_MINUTE);
      }

      {
        auto instrument = candlesRequest->add_instruments();
        instrument->set_figi(appleFigi);
        instrument->set_interval(contract::v1::SubscriptionInterval::SUBSCRIPTION_INTERVAL_ONE_MINUTE);
      }
      

      auto result = co_await marketDataStreamService.start(method, request);
      errorCode = result.status.error_code();
    }
  },
  boost::asio::detached);

  grpcExecutionContext.run();

  return errorCode;
}