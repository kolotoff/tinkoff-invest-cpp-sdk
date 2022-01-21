#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <google/protobuf/util/json_util.h>

#include <agrpc/asioGrpc.hpp>

#include <Credentials.hpp>

template <class Service>
class Request
{
public:
  using Stub = Service::Stub;
  template <class Response>
  using Reader = std::unique_ptr<grpc::ClientAsyncResponseReader<Response>>;

  struct Limits
  {
    int remaining; //x-ratelimit-remaining Ч количество оставшихс€ запросов данного типа в минуту.
    int secondsToReset; //x-ratelimit-reset Ч врем€ в секундах до обнулени€ счЄтчика запросов.
  };

  template <class Response>
  struct Result
  {
    grpc::Status status;
    Limits limits;
    std::string trackingId;
    Response response;

    bool success() const { return status.ok(); }
  };

public:
  Request(const Credentials& credentials)
  {
    clientContext_.AddMetadata("authorization", "Bearer " + credentials.token());
    clientContext_.AddMetadata("x-app-name", "kolotoff.tinkoff-invest-cpp-sdk");
  }

  ~Request() = default;

  template<class TRequest, class Response>
  boost::asio::awaitable<Result<Response>> execute(const std::unique_ptr<Stub>& stub
    , Reader<Response>(Stub::* method)(grpc::ClientContext* context, const TRequest& request, grpc::CompletionQueue*)
    , TRequest& request)
  {
    Result<Response> result;

    const auto executor = co_await boost::asio::this_coro::executor;
    auto reader = ((*stub).*method)(&clientContext_, request, agrpc::get_completion_queue(executor));
    co_await agrpc::finish(*reader, result.response, result.status);


    std::cout << request.FullMessageName() << std::endl;

    //Can't use initializer-list due existing constructor
    static const auto printOptions = []
    {
      google::protobuf::util::JsonPrintOptions options;
      options.add_whitespace = true;
      options.always_print_primitive_fields = true;
      options.always_print_enums_as_ints = false;
      options.preserve_proto_field_names = true;
      return options;
    }();

    std::string json;
    google::protobuf::util::MessageToJsonString(request, &json, printOptions);
    std::cout << json << std::endl;

    printMetadata();
    extractMetadata(result.limits, result.trackingId);
 
    if (!result.status.ok())
    {
      std::cout << "Error " << std::dec << result.status.error_code() << ": " << result.status.error_message() << "; "
        << result.status.error_details();
    }
    else
    {   
      json.clear();
      google::protobuf::util::MessageToJsonString(result.response, &json, printOptions);
      std::cout << std::endl << "Response: " << std::endl << json << std::endl;
    }
    
    co_return result;
  }


  void printMetadata()
  {
    std::cout << "Metadata: " << std::endl;
    printMetadata(clientContext_.GetServerInitialMetadata());
  }

private:

  void extractMetadata(Limits& limits, std::string& trackingId)
  {
    auto& metadata = clientContext_.GetServerInitialMetadata();

    auto extractInt = [&](const std::string& key, int& destination)
    {
      destination = {};
      try
      {
        auto iterator = metadata.find(key);
        if (iterator != metadata.end())
        {
          destination = std::atoi(iterator->second.data());
        }
      }
      catch (...) { }
    };

    auto extractString = [&](const std::string& key, std::string& destination)
    {
      destination = {};
      try
      {
        auto iterator = metadata.find(key);
        if (iterator != metadata.end())
        {
          destination = std::string(iterator->second.data(), iterator->second.length());
        }
      }
      catch (...) {}
    };

    extractString("x-tracking-id", trackingId);
    extractInt("x-ratelimit-remaining", limits.remaining);
    extractInt("x-ratelimit-reset", limits.secondsToReset);
  }

  void printMetadata(const auto& metadata)
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
  }

private:
  grpc::ClientContext clientContext_ {};
  
};

#endif // !REQUEST_HPP
