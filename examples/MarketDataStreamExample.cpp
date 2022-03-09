#include "Examples.hpp"

#include <MarketDataStream.hpp>


int main(int /*argc*/, const char** /*argv*/)
{
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif

  Credentials credentials;
  if (!credentials.load())
  {
    return -1;
  }
  agrpc::GrpcContext grpcExecutionContext { std::make_unique<grpc::CompletionQueue>() };

  int errorCode = 0;

  TinkoffInvest::MarketDataStream marketDataStream(credentials);

  static const std::string appleFigi = "BBG000B9XRY4"; //Apple
  static const std::string mvideoFigi = "BBG004S68CP5"; //MVID
  static const std::string spyf3_22 = "FUTSPYF03220"; //SPYF-3.22

  marketDataStream.addInstrument(appleFigi);
  marketDataStream.addInstrument(mvideoFigi);
  marketDataStream.addInstrument(spyf3_22);

  marketDataStream.start(grpcExecutionContext);

  grpcExecutionContext.run();

  return errorCode;
}