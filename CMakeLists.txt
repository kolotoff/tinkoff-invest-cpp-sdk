cmake_minimum_required (VERSION 3.20)

project("tinkoff-invest-cpp-sdk" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)

if(MSVC)
    set(Protobuf_USE_STATIC_LIBS ON)
endif()

find_package(Boost 1.78.0 REQUIRED)
find_package(OpenSSL 1.1.0 REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Protobuf_INCLUDE_DIR})
include_directories("${CMAKE_CURRENT_BINARY_DIR}/")

message("PROTOBUF Found: ${Protobuf_FOUND} ${Protobuf_VERSION}")

add_subdirectory(deps/asio-grpc)
include(deps/asio-grpc/cmake/AsioGrpcProtobufGenerator.cmake)
include(deps/asio-grpc/cmake/AsioGrpcCompileOptions.cmake)

include_directories("src") 

if(MSVC)
    add_compile_options(/bigobj)
    add_compile_options(/EHsc)
    add_compile_options(/MP)
endif()

add_library(protos-compile-options OBJECT)

target_compile_options(
    protos-compile-options
    INTERFACE $<$<CXX_COMPILER_ID:MSVC>:
              /external:I
              $<TARGET_PROPERTY:protobuf::libprotobuf,INTERFACE_INCLUDE_DIRECTORIES>
              /external:W0
              /external:templates-
              /W4>
              $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall
              -Wextra
              -pedantic-errors>
              $<$<CXX_COMPILER_ID:Clang>:-Wno-self-move>)

target_compile_definitions(
    protos-compile-options
    INTERFACE $<$<CXX_COMPILER_ID:MSVC>:
              BOOST_ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT
              BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT
              BOOST_ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT
              BOOST_ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT
              BOOST_ASIO_HAS_DEDUCED_PREFER_MEMBER_TRAIT
              _WIN32_WINNT=0x0A00 # Windows 10
              WINVER=0x0A00
              BOOST_ASIO_NO_DEPRECATED)

set(VAR_GENERATED_PROTOS_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/")
set(VAR_GENERATED_PROTOS_OUT_DIR "${VAR_GENERATED_PROTOS_INCLUDE_DIR}/protos")

asio_grpc_protobuf_generate(
    GENERATE_GRPC
    TARGET protos-compile-options
    OUT_VAR OUT_VAR_GENERATED_SOURCES
    OUT_DIR "${VAR_GENERATED_PROTOS_OUT_DIR}"
    PROTOS "${CMAKE_CURRENT_SOURCE_DIR}/src/protos/helloworld.proto"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/protos/example.proto")

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

if(MSVC)
    set(OPENSSL_PATH_STR ${OPENSSL_ROOT_DIR}/bin)
    set(PATH_STR "PATH=%PATH%" ${OPENSSL_PATH_STR})
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_ENVIRONMENT "${PATH_STR}")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE protos-compile-options)
target_link_libraries(${PROJECT_NAME} PRIVATE gRPC::grpc++ gRPC::grpc++_reflection)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${PROJECT_NAME} PRIVATE asio-grpc::asio-grpc)