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

include_directories("src") 

if(MSVC)
    add_compile_options(/bigobj)
    add_compile_options(/EHsc)
    add_compile_options(/MP)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")
endif()

add_library(tinkoff_api_protos OBJECT)

target_compile_options(
    tinkoff_api_protos
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
    tinkoff_api_protos
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
    TARGET tinkoff_api_protos
    OUT_DIR "${GENERATED_PROTOS_OUT_DIR}"
    PROTOS ${InvestApiProtos})

include(cmake/RestoreProtosPackagePublicKeyword.cmake)

function(tinkoff_invest_sdk_add_credentials_files out_folder)
    set(rootCertFileName "roots.pem")
    set(tokenFileName "token.txt")
    set(accountFileName "account.txt")

    if(NOT EXISTS "${out_folder}/${rootCertFileName}")
        file(COPY "${GRPC_ROOT}/share/grpc/${rootCertFileName}" DESTINATION ${out_folder})
    endif()

    if(NOT EXISTS "${out_folder}/${tokenFileName}")
        file(WRITE "${out_folder}/${tokenFileName}" "place_your_t_token_here")
    endif()

    if(NOT EXISTS "${out_folder}/${accountFileName}")
        file(WRITE "${out_folder}/${accountFileName}" "place_your_account_id_here")
    endif()
endfunction()

function(tinkoff_invest_sdk_add_example example_name)
    add_executable(${example_name})

    target_sources(${example_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/examples/${example_name}.cpp")

    if(MSVC)
        set(OPENSSL_PATH_STR ${OPENSSL_ROOT_DIR}/bin)
        set(PATH_STR "PATH=%PATH%" ${OPENSSL_PATH_STR})
        set_target_properties(${example_name} PROPERTIES VS_DEBUGGER_ENVIRONMENT "${PATH_STR}")
        set_target_properties(${example_name} PROPERTIES LINK_FLAGS "/INCREMENTAL")

        #For MSVC avoid default /Debug or /Release created folder
        set_target_properties(${example_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    target_link_libraries(${example_name} PRIVATE tinkoff_api_protos)
    target_link_libraries(${example_name} PRIVATE gRPC::grpc++ gRPC::grpc++_reflection)
    target_link_libraries(${example_name} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(${example_name} PRIVATE asio-grpc::asio-grpc)
endfunction()

tinkoff_invest_sdk_add_credentials_files(${CMAKE_CURRENT_BINARY_DIR})

tinkoff_invest_sdk_add_example(GetAccountsExample)
tinkoff_invest_sdk_add_example(GetUserTariffExample)
tinkoff_invest_sdk_add_example(MarketDataStreamExample)