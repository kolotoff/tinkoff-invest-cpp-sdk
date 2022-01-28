#ifndef SERVICE_HPP
#define SERVICE_HPP

#include <agrpc/asioGrpc.hpp>

#include <GprcUtil.hpp>

template <class GprcService>
class Service
{
public:
  using Method = GprcService::Stub;

  template <class Response>
  using Reader = std::unique_ptr<grpc::ClientAsyncResponseReader<Response>>;
  template <class Response>
  using Result = GprcUtil::Result<Response>;

public:
  Service(const Credentials& credentials)
    : credentials_(credentials)
  {
    stub_ = GprcUtil::makeStub<GprcService>(credentials_);
  }

  ~Service() = default;

  template<class TRequest, class Response>
  TRequest makeRequest(Reader<Response>(Method::*)(grpc::ClientContext*, const TRequest&, grpc::CompletionQueue*))
  {
    TRequest request;
    return request;
  }

  template<class TRequest, class Response>
  boost::asio::awaitable<Result<Response>> execute(Reader<Response>(Method::*method)(grpc::ClientContext*, const TRequest&, grpc::CompletionQueue*)
    , const TRequest& request)
  {
    Result<Response> result;

    auto clientContext = GprcUtil::makeContext(credentials_);
    
    const auto executor = co_await boost::asio::this_coro::executor;
    auto reader = ((*stub_).*method)(clientContext.get(), request, agrpc::get_completion_queue(executor));
    co_await agrpc::finish(*reader, result.response, result.status);


    GprcUtil::logRequest(request);
    GprcUtil::populateResult(result, clientContext);
    GprcUtil::logResult(result);

    co_return result;
  }

private:
  const Credentials& credentials_;
  std::unique_ptr<Method> stub_;

};

#endif // !SERVICE_HPP
