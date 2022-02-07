cmake_minimum_required (VERSION 3.20)

project("tinkoff-invest-cpp-sdk" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if(MSVC)
    set(Protobuf_USE_STATIC_LIBS ON)
endif()

find_package(Boost 1.78.0 REQUIRED)
find_package(OpenSSL 1.1.0 REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Protobuf_INCLUDE_DIR})

message("PROTOBUF Found: ${Protobuf_FOUND} ${Protobuf_VERSION}")

add_subdirectory(deps/asio-grpc)
include(deps/asio-grpc/cmake/AsioGrpcCompileOptions.cmake)

include_directories("src") 

if(MSVC)
    add_compile_options(/bigobj)
    add_compile_options(/EHsc)
    add_compile_options(/MP)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")
endif()

add_library(protos OBJECT)

target_compile_options(
    protos
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
    protos
    INTERFACE $<$<CXX_COMPILER_ID:MSVC>:
              BOOST_ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT
              BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT
              BOOST_ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT
              BOOST_ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT
              BOOST_ASIO_HAS_DEDUCED_PREFER_MEMBER_TRAIT
              _WIN32_WINNT=0x0A00 # Windows 10
              WINVER=0x0A00
              BOOST_ASIO_NO_DEPRECATED)

set(GENERATED_PROTOS_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/protos")

include(cmake/GetInvestApiProtosList.cmake)

include(cmake/GrpcProtobufGenerate.cmake)

grpc_protobuf_generate(
    GENERATE_GRPC
    TARGET protos
    OUT_DIR "${GENERATED_PROTOS_OUT_DIR}"
    PROTOS ${InvestApiProtos})

include(cmake/RestoreProtosPackagePublicKeyword.cmake)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

if(MSVC)
    set(OPENSSL_PATH_STR ${OPENSSL_ROOT_DIR}/bin)
    set(PATH_STR "PATH=%PATH%" ${OPENSSL_PATH_STR})
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_ENVIRONMENT "${PATH_STR}")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE protos)
target_link_libraries(${PROJECT_NAME} PRIVATE gRPC::grpc++ gRPC::grpc++_reflection)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${PROJECT_NAME} PRIVATE asio-grpc::asio-grpc)