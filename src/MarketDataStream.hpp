#ifndef TINKOFFINVEST_MARKETDATASTREAM_HPP
#define TINKOFFINVEST_MARKETDATASTREAM_HPP

#include <StreamingService.hpp>

#include <protos/marketdata.grpc.pb.h>

namespace TinkoffInvest
{
  namespace Contract = tinkoff::Public::invest::api::contract::v1;

class MarketDataStream
{

public:
  using Figi = std::string;
  using MarketDataStreamService = StreamingService<Contract::MarketDataStreamService>;
  using Response = Contract::MarketDataResponse;
   
public:
  MarketDataStream(const Credentials& credentials)
    : service_(credentials)
  {

  }

  ~MarketDataStream() = default;

  void addInstrument(const Figi& figi)
  {
    instruments_.push_back(figi);
  }

  void start(agrpc::GrpcContext& executionContext)
  {
    boost::asio::co_spawn(executionContext, [&]() -> boost::asio::awaitable<void>
    {
      auto request = service_.makeRequest(method_);

      auto candlesRequest = request.mutable_subscribe_candles_request();
      candlesRequest->set_subscription_action(Contract::SubscriptionAction::SUBSCRIPTION_ACTION_SUBSCRIBE);

      for (const auto& figi : instruments_)
      {
        auto instrument = candlesRequest->add_instruments();
        instrument->set_figi(figi);
        instrument->set_interval(Contract::SubscriptionInterval::SUBSCRIPTION_INTERVAL_ONE_MINUTE);
      }

      auto stream = co_await service_.start(method_, request);
      if (stream)
      {
        while (stream->ready())
        {
          auto [readOk, response] = co_await stream->read();
          if (readOk)
          {
            parse(response);
          }
        }
      }
    },
    boost::asio::detached);
  }

private:
  void parse(const Response& response)
  {
    switch (response.payload_case())
    {
      case Response::kSubscribeCandlesResponse:
      break;

      case Response::kSubscribeOrderBookResponse:
      break;

      case Response::kSubscribeTradesResponse:
      break;

      case Response::kSubscribeInfoResponse:
      break;

      case Response::kCandle:
      break;

      case Response::kTrade:
      break;

      case Response::kOrderbook:
      break;

      case Response::kTradingStatus:
      break;

      case Response::kPing:
      break;

      case Response::PAYLOAD_NOT_SET:
      break;
    }
  }

private:
  inline const static auto method_ = &MarketDataStreamService::Method::AsyncMarketDataStream;

  MarketDataStreamService service_;
  std::list<Figi> instruments_;

};

} // namespace TinkoffInvest

#endif // !TINKOFFINVEST_MARKETDATASTREAM_HPP
