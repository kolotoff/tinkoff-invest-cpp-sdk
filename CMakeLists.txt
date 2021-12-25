cmake_minimum_required (VERSION 3.13)

project("tinkoff-invest-cpp-sdk" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.78.0 REQUIRED)
find_package(OpenSSL 1.1.0 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(deps/asio-grpc)
find_package(asio-grpc)

include_directories("src") 

function(asio_grpc_add_example_protos _asio_grpc_name)
    include(deps/asio-grpc/cmake/AsioGrpcProtobufGenerator.cmake)

    set(ASIO_GRPC_GENERATED_EXAMPLE_PROTOS_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
    set(ASIO_GRPC_GENERATED_EXAMPLE_PROTOS_OUT_DIR "${ASIO_GRPC_GENERATED_EXAMPLE_PROTOS_INCLUDE_DIR}/protos")

    asio_grpc_protobuf_generate(
        GENERATE_GRPC
        OUT_VAR ASIO_GRPC_GENERATED_EXAMPLE_SOURCES
        OUT_DIR "${ASIO_GRPC_GENERATED_EXAMPLE_PROTOS_OUT_DIR}"
        PROTOS "${CMAKE_CURRENT_LIST_DIR}/protos/helloworld.proto" "${CMAKE_CURRENT_LIST_DIR}/protos/example.proto")

    add_library(asio-grpc-${_asio_grpc_name} OBJECT)

    target_sources(asio-grpc-${_asio_grpc_name} PRIVATE ${ASIO_GRPC_GENERATED_EXAMPLE_SOURCES})

    target_include_directories(
        asio-grpc-${_asio_grpc_name} PUBLIC $<BUILD_INTERFACE:${ASIO_GRPC_GENERATED_EXAMPLE_PROTOS_INCLUDE_DIR}>
                                            $<INSTALL_INTERFACE:include>)

    target_link_libraries(asio-grpc-${_asio_grpc_name} PRIVATE asio-grpc-common-compile-options)
endfunction()

if(MSVC)
	add_compile_options(/bigobj)
	add_compile_options(/EHsc)
	add_compile_options(/MP)
	add_compile_options(/Gm)
endif()

add_executable (${PROJECT_NAME} "src/main.cpp")

target_link_libraries (${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${PROJECT_NAME} asio-grpc::asio-grpc)

asio_grpc_add_example_protos(example-protos)

if(MSVC)
	set(OPENSSL_PATH_STR ${OPENSSL_ROOT_DIR}/bin)
	set(PATH_STR "PATH=%PATH%" ${OPENSSL_PATH_STR})
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_ENVIRONMENT "${PATH_STR}")
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL")
endif()