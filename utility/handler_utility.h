#ifndef HANDLER_UTILITY_H
#define HANDLER_UTILITY_H

#include <unordered_map>
#include "boost_header.h"

// Typedef for request handler function
using RequestHandler = std::function<void(beast::tcp_stream&, http::request<http::string_body>&)>;

// dummmy code




// Map to associate URIs with handlers
inline std::unordered_map<std::string, RequestHandler> handlers = {};

// Handler for unknown endpoints
inline void handle_not_found(beast::tcp_stream& stream, http::request<http::string_body>& req)
{
    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::content_type, "application/json");
    res.body() = "{}";
    res.prepare_payload();
    http::write(stream, res);
}

inline void parsing_error(beast::tcp_stream& stream, http::request<http::string_body>& req)
{
    http::response<http::string_body> res{http::status::bad_request, req.version()};
    res.set(http::field::content_type, "application/json");
    res.body() = "{}";
    res.prepare_payload();
    http::write(stream, res);
}

inline void unknown_error(beast::tcp_stream& stream, http::request<http::string_body>& req)
{
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::content_type, "application/json");
    res.body() = "{}";
    res.prepare_payload();
    http::write(stream, res);
}

// This function processes the HTTP request
inline void handle_request(beast::tcp_stream& stream, http::request<http::string_body>&& req)
{
    std::string path = std::string(req.target());
    auto handler = handlers.find(path);
    if (handler != handlers.end())
    {
        // Found the handler for the path
        handler->second(stream, req);
    }
    else
    {
        // No handler found for the path
        handle_not_found(stream, req);
    }
}


#endif //HANDLER_UTILITY_H
