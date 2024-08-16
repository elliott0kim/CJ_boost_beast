#ifndef SESSION_H
#define SESSION_H

#include "boost_header.h"
//#include "handler_uri.h"
#include <iostream>

// Asynchronous session handler
class session : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket);
    ~session();
    void start();

private:
    void do_read();
    void handle_request();

    beast::flat_buffer buffer_;
    http::request<http::string_body> request_;
    beast::tcp_stream stream_;
};

#endif //SESSION_H
