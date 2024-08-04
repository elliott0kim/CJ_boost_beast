#ifndef HANDLER_URI_H
#define HANDLER_URI_H

#include "handler_utility.h"

// Handler for the /getHeartRate endpoint
void handle_get_heart_rate(beast::tcp_stream& stream, http::request<http::string_body>& req);

// Handler for the /setHeartRate endpoint
void handle_set_heart_rate(beast::tcp_stream& stream, http::request<http::string_body>& req);

// Handler for the /workStatusOff endpoint
void handle_work_status_off(beast::tcp_stream& stream, http::request<http::string_body>& req);

// Handler for the /lastHeartRate endpoint
void handle_last_heart_rate(beast::tcp_stream& stream, http::request<http::string_body>& req);

// api insert to handler unorderedmap
void handler_insert();

// void handle_get_heart_rate(beast::tcp_stream& stream, http::request<http::string_body>& req);
//
// void handle_set_heart_rate(beast::tcp_stream& stream, http::request<http::string_body>& req);
//
// void query_get_heart_rate(std::string& user_id , std::string& heart_rate_date);
//
// std::string blobToString(const std::vector<unsigned char>& data);
//
// #include "boost_header.h"
// #include "handler_uri.h"
// #include <iostream>
// #include <unordered_map>
// #include <functional>
//
// // Typedef for request handler function
// using request_handler = std::function<void(beast::tcp_stream&, http::request<http::string_body>&)>;
//
// // Map to associate URIs with handlers
// inline std::unordered_map<std::string, request_handler> handlers;
//
// inline void handle_not_found(beast::tcp_stream& stream, http::request<http::string_body>& req)
// {
//     http::response<http::string_body> res{http::status::not_found, req.version()};
//     res.set(http::field::content_type, "text/plain");
//     res.body() = "404 Not Found: The requested resource could not be found.";
//     res.prepare_payload();
//     http::write(stream, res);
// }
//
// inline void handle_request(beast::tcp_stream& stream, http::request<http::string_body>&& req)
// {
//     std::string path = std::string(req.target());
//     auto handler = handlers.find(path);
//     if (handler != handlers.end())
//     {
//         // Found the handler for the path
//         handler->second(stream, req);
//     }
//     else
//     {
//         // No handler found for the path
//         handle_not_found(stream, req);
//     }
// }
//
// inline void initialize_handlers()
// {
//     handlers.insert({"/getHeartRate", handle_get_heart_rate});
//     handlers.insert({"/setHeartRate", handle_set_heart_rate});
// }

#endif //HANDLER_URI_H
