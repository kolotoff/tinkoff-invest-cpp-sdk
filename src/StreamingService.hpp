#ifndef STREAMINGSERVICE_HPP
#define STREAMINGSERVICE_HPP

#include <functional>

#include <agrpc/asioGrpc.hpp>

#include <GprcUtil.hpp>

template <class GprcService>
class StreamingService
{
public:
  using Method = GprcService::Stub;

  template <class Request, class Response>
  using Stream = std::unique_ptr<grpc::ClientAsyncReaderWriter<Request, Response>>;
  template <class Response>
  using Result = GprcUtil::Result<Response>;

public:
  StreamingService(const Credentials& credentials)
    : credentials_(credentials)
  {
    stub_ = GprcUtil::makeStub<GprcService>(credentials_);
  }

  ~StreamingService() = default;

  template<class Request, class Response>
  Request makeRequest(Stream<Request, Response>(Method::*)(grpc::ClientContext*, grpc::CompletionQueue*, void*))
  {
    Request request;
    return request;
  }

  template<class Request, class Response>
  boost::asio::awaitable<Result<Response>> start(Stream<Request, Response>(Method::* method)(grpc::ClientContext*, grpc::CompletionQueue*, void*)
    , const Request& request/*, std::function<void(const Response&)> parser*/)
  {
    Result<Response> result;

    auto clientContext = GprcUtil::makeContext(credentials_);

    const auto executor = co_await boost::asio::this_coro::executor;

    Stream<Request, Response> stream;

    bool requestOk = co_await agrpc::request(method, *stub_, *clientContext, stream);
 
    if (requestOk)
    {
      bool writeOk = co_await agrpc::write(*stream, request);
      if (writeOk)
      {
        bool readOk = true;
        while (readOk)
        {
          Response response;
          readOk = co_await agrpc::read(*stream, response);
          if (readOk)
          {
            auto json = GprcUtil::messageToString(response);
            std::cout << std::endl << "Response: " << std::endl << json << std::endl;
            //parser(response);
          }
        }
      }
    }

    co_await agrpc::writes_done(*stream);
    bool finishOk = co_await agrpc::finish(*stream, result.status);

    //auto stream = ((*stub_).*method)(clientContext.get(), request, agrpc::get_completion_queue(executor));
    //co_await agrpc::request(*stream, result.response, result.status);

    GprcUtil::logRequest(request);
    GprcUtil::populateResult(result, clientContext);
    GprcUtil::logResult(result);

    co_return result;
  }

private:
  const Credentials& credentials_;
  std::unique_ptr<Method> stub_;

};

#endif // !STREAMINGSERVICE_HPP
