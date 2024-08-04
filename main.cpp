#include <iostream>

#include "define_utility.h"
#include "handler_uri.h"
#include "session.h"
#include "do_connection.h"


int main()
{
    try
    {
        int num_threads = std::thread::hardware_concurrency();

        handler_insert();

        net::io_context ioc{num_threads}; // Number of threads in the pool

        tcp::acceptor acceptor{ioc, {tcp::v4(), SERVER_PORT}};

        // Start accepting connections
        //net::socket_base::reuse_address option(true);
        net::socket_base::reuse_address option(true);
        acceptor.set_option(option);

        do_accept(acceptor, ioc);

        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&ioc] {
                ioc.run();
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }
    catch (std::exception const& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
