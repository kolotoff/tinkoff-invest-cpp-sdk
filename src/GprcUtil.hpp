#ifndef GPRCUTIL_HPP
#define GPRCUTIL_HPP

#include <google/protobuf/util/json_util.h>
#include <grpcpp/create_channel.h>

#include <Credentials.hpp>

class GprcUtil
{

public:
  struct Limits
  {
    int remaining; //x-ratelimit-remaining — количество оставшихся запросов данного типа в минуту.
    int secondsToReset; //x-ratelimit-reset — время в секундах до обнуления счётчика запросов.
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
  inline static auto makeContext(const Credentials& credentials)
  {
    std::unique_ptr<grpc::ClientContext> context(new grpc::ClientContext()) ;
    context->AddMetadata("authorization", "Bearer " + credentials.token());
    context->AddMetadata("x-app-name", appName_);
    return context;
  }

  template <class GprcService>
  inline static auto makeStub(const Credentials& credentials)
  {
    grpc::SslCredentialsOptions sslOptions;
    sslOptions.pem_root_certs = credentials.certs();

    auto sslCredentials = grpc::SslCredentials(sslOptions);

    return GprcService::NewStub(grpc::CreateChannel(host_, sslCredentials));
  }

  template <class Message>
  inline static std::string messageToString(const Message& message)
  {
    std::string json;
    google::protobuf::util::MessageToJsonString(message, &json, printOptions_);
    return json;
  }

  template <class Response>
  inline static void logResult(const Result<Response>& result)
  {
    if (!result.status.ok())
    {
      std::cout << "Error " << std::dec << result.status.error_code() << ": " << result.status.error_message() << "; "
        << result.status.error_details();
    }
    else
    {
      auto json = messageToString(result.response);
      std::cout << std::endl << "Response: " << std::endl << json << std::endl;
    }
  }

  template<class Request>
  inline static void logRequest(const Request& request)
  {
    std::cout << request.GetTypeName() << std::endl;
    std::cout << messageToString(request) << std::endl;
  }

  template <class Response>
  inline static void populateResult(Result<Response>& result, const std::unique_ptr<grpc::ClientContext>& clientContext)
  {
    std::cout << "Metadata: " << std::endl;
    printMetadata(clientContext->GetServerInitialMetadata());
    extractMetadata(*clientContext, result.limits, result.trackingId);
  }

private:
  inline static void extractMetadata(const grpc::ClientContext& context, Limits& limits, std::string& trackingId)
  {
    auto& metadata = context.GetServerInitialMetadata();

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
      catch (...) {}
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

  inline static void printMetadata(const auto& metadata)
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

  //Can't use initializer-list due existing constructor
  inline static const auto printOptions_ = []
  {
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.always_print_enums_as_ints = false;
    options.preserve_proto_field_names = true;
    return options;
  }();


  inline static const auto host_ = "invest-public-api.tinkoff.ru:443";
  inline static const auto appName_ = "kolotoff.tinkoff-invest-cpp-sdk";

private:
  GprcUtil() = default;
  ~GprcUtil() = delete;

};


#endif // !GPRCUTIL_HPP
