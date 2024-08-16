########################################################################################################
#                                                                                                      #
#  Project Name : CJ Watch Server                                                                      #
#  Author : KIM DONG HYUN                                                                              #
#  Current Organization : INHA UNIV                                                                    #
#  Previous Organization :                                                                             #
#    Chips & Media - Reference Software Developer Assistant                      (Contract Employee)   #
#    DreamSecurity - Linux Desktop Application Developer for PKI Infrastructure  (Permanent Employee)  #
#                                                                                                      #
#  Description :                                                                                       #
#    Most services are provided by the Spring Boot server,                                             #
#    and this project is solely for storing members' heart rates from Galaxy Watch into MySQL          #
#    and providing heart rate data when needed.                                                        #
#    To handle large traffic, a partially Microservices Architecture (MSA) is implemented.             #
#                                                                                                      #
#                                                                                                      #
#  If you have any other questions, please contact us at the email address below.                      #
#    -> jeremy22283@gmail.com                                                                          #
#                                                                                                      #
########################################################################################################



cmake_minimum_required(VERSION 3.28)
project(CJ_boost_beast)

# CMAKE version
set(CMAKE_CXX_STANDARD 17)

# message for current dir
# for debug
message(STATUS "project root dir path : ${CMAKE_SOURCE_DIR}")

# boost beast tools
set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/boost_tools")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
find_package(Boost 1.70 REQUIRED COMPONENTS system filesystem)
include_directories(${BOOST_INCLUDEDIR})

# jwt tools
set(JWT_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")
set(JWT_INCLUDEDIR "${JWT_ROOT}/installed/x64-osx/include")
include_directories(${JWT_INCLUDEDIR})
set(OpenSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/openssl/3.3.1")
set(OpenSSL_LIBRARIES "${OpenSSL_ROOT_DIR}/lib")
set(OpenSSL_INCLUDE_DIR "${OpenSSL_ROOT_DIR}/include")
find_package(OpenSSL REQUIRED)
include_directories(${OpenSSL_INCLUDE_DIR})
link_directories(${OpenSSL_LIBRARIES})

# mysql tools
set(MYSQL_ROOT "${CMAKE_SOURCE_DIR}/mysql-connector-c++-8.1.0")
set(MYSQL_INCLUDE_DIR "${MYSQL_ROOT}/include")
set(MYSQL_LIBRARIES "${MYSQL_ROOT}/lib64")
#find_package(mysql-concpp REQUIRED COMPONENTS XDevAPI)
include_directories(${MYSQL_INCLUDE_DIR})
link_directories(${MYSQL_LIBRARIES})

# rapidjson-master tools
set(RAPIDJSON_ROOT "${CMAKE_SOURCE_DIR}/rapidjson-master/include")
include_directories(${RAPIDJSON_ROOT})

# header path
set(HEADER "${CMAKE_SOURCE_DIR}")
include_directories("${HEADER}/data_collector")
include_directories("${HEADER}/database_connector")
include_directories("${HEADER}/handler_uri")
include_directories("${HEADER}/message")
include_directories("${HEADER}/token_parser")
include_directories("${HEADER}/session")
include_directories("${HEADER}/utility")

add_executable(CJ_boost_beast main.cpp
        utility/define_utility.h
        boost_header.h
        session/session.h
        session/session.cpp
        utility/handler_utility.h
        utility/make_json_format.h
        message/message_to_define.h
        token_parser/token_parser.h
        token_parser/token_parser.cpp
        boost_tools/include/boost/beast/core/tcp_stream.hpp
        handler_uri/handler_uri.h
        handler_uri/handler_uri.cpp
        do_connection.h
        data_collector/data_collector.h
        data_collector/data_collector.cpp
        database_connector/database_connector.h
        database_connector/database_connector.cpp)

# Boost 라이브러리 링크
target_link_libraries(CJ_boost_beast PRIVATE OpenSSL::SSL OpenSSL::Crypto
        libcrypto.3.dylib
        libfido2.1.8.0.dylib
        libssl.3.dylib
        libmysqlcppconn8.2.8.1.0.dylib)