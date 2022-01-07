#include "helper/helper.hpp"
#include "protos/helloworld.grpc.pb.h"


#include <agrpc/asioGrpc.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

int main(int argc, const char** argv)
{
    const auto port = 443
    const auto host = "invest-public-api.tinkoff.ru";

    grpc::Status status;

    // begin-snippet: client-side-helloworld
    const auto stub = helloworld::Greeter::NewStub(grpc::CreateChannel(host, grpc::InsecureChannelCredentials()));
    agrpc::GrpcContext grpc_context{std::make_unique<grpc::CompletionQueue>()};

    boost::asio::co_spawn(
        grpc_context,
        [&]() -> boost::asio::awaitable<void>
        {
            grpc::ClientContext client_context;
            helloworld::HelloRequest request;
            request.set_name("world");
            std::unique_ptr<grpc::ClientAsyncResponseReader<helloworld::HelloReply>> reader =
                stub->AsyncSayHello(&client_context, request, agrpc::get_completion_queue(grpc_context));
            helloworld::HelloReply response;
            co_await agrpc::finish(*reader, response, status);
        },
        boost::asio::detached);

    grpc_context.run();
    // end-snippet

    abort_if_not(status.ok());
}