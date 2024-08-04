#ifndef DO_CONNECTION_H
#define DO_CONNECTION_H

#include <functional>
#include <thread>

#include "boost_header.h"
#include "session.h"

// Function to process connections
inline void do_session(tcp::socket socket)
{
    std::make_shared<session>(std::move(socket))->start();
}

// Function to asynchronously accept connections
inline void do_accept(tcp::acceptor& acceptor, net::io_context& ioc)
{
    acceptor.async_accept(
        [&](beast::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::thread{[s = std::move(socket)]() mutable {
                    do_session(std::move(s));
                }}.detach();
            }

            // Accept the next connection
            do_accept(acceptor, ioc);
        });
}

#endif //DO_CONNECTION_H
