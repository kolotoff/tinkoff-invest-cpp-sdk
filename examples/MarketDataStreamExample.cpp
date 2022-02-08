#include "Examples.hpp"

#include <protos/marketdata.grpc.pb.h>

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

  using MarketDataStreamService = StreamingService<contract::v1::MarketDataStreamService>;
  MarketDataStreamService marketDataStreamService(credentials);

  boost::asio::co_spawn(grpcExecutionContext, [&]() -> boost::asio::awaitable<void>
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
    
  },
  boost::asio::detached);

  grpcExecutionContext.run();

  return errorCode;
}