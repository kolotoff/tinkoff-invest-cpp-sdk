#ifndef STREAMINGSERVICE_HPP
#define STREAMINGSERVICE_HPP

#include <agrpc/asioGrpc.hpp>

#include <GprcUtil.hpp>

template <class GprcService>
class StreamingService
{
public:
  using Method = GprcService::Stub;

  template <class Request, class Response>
  using ReaderWriter = std::unique_ptr<grpc::ClientAsyncReaderWriter<Request, Response>>;

  template <class Response>
  using Result = GprcUtil::Result<Response>;

  template <class Request, class Response>
  class Stream
  {
    friend class StreamingService;

    GprcUtil::Limits limits;
    std::string trackingId;

  public:
    Stream(const Credentials& credentials)
    {
      stub_ = GprcUtil::makeStub<GprcService>(credentials);
      clientContext_ = GprcUtil::makeContext(credentials);;
    }
    ~Stream() = default;

    bool ready() const { return ready_; }

    boost::asio::awaitable<void> finish()
    {
      co_await agrpc::writes_done(*readerWriter_);
      grpc::Status status;
      bool finishOk = co_await agrpc::finish(*readerWriter_, status);
      ready_ = false;
    }


    boost::asio::awaitable<std::pair<bool, Response>> read()
    {
      std::pair<bool, Response> result;

      bool readOk = co_await agrpc::read(*readerWriter_, result.second);
      if (readOk)
      {
        auto json = GprcUtil::messageToString(result.second);
        std::cout << std::endl << "Response: " << std::endl << json << std::endl;
      }

      result.first = readOk;

      co_return result;
    }

  private:
    template<class Request, class Method>
    boost::asio::awaitable<bool> start(Method method, const Request& request)
    {
      bool requestOk = co_await agrpc::request(method, *stub_, *clientContext_, readerWriter_);

      if (requestOk)
      {
        ready_ = co_await agrpc::write(*readerWriter_, request);
      }

      co_return ready_;
    }

  private:
    std::unique_ptr<Method> stub_;
    std::unique_ptr<grpc::ClientContext> clientContext_;
    ReaderWriter<Request, Response> readerWriter_;
    bool ready_ = false;

  };

public:
  StreamingService(const Credentials& credentials)
    : credentials_(credentials)
  {
  }

  ~StreamingService() = default;

  template<class Request, class Response>
  Request makeRequest(ReaderWriter<Request, Response>(Method::*)(grpc::ClientContext*, grpc::CompletionQueue*, void*))
  {
    Request request;
    return request;
  }

  template<class Request, class Response>
  boost::asio::awaitable<std::unique_ptr<Stream<Request, Response>>> start(ReaderWriter<Request, Response>(Method::* method)(grpc::ClientContext*, grpc::CompletionQueue*, void*)
    , const Request& request)
  {
    auto stream = std::make_unique<Stream<Request, Response>>(credentials_);

    GprcUtil::logRequest(request);

    if (co_await stream->start(method, request))
    {
      co_return std::move(stream);
    }

    co_return nullptr;
  }

private:
  const Credentials& credentials_;

};

#endif // !STREAMINGSERVICE_HPP
