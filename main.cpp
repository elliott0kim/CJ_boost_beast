#include <iostream>

#include "data_collector.h"
#include "define_utility.h"
#include "handler_uri.h"
#include "session.h"
#include "do_connection.h"

#include "database_connector.h"

using namespace std::chrono;

void save_user_heartrate_data()
{
    std::cout << "save_user_heartrate_data executed at: " << system_clock::now().time_since_epoch().count() << std::endl;
    date_passed();
}

class day_data_manager {
public:
    day_data_manager(boost::asio::io_context& io) : timer_(io)
    {
        timer_.async_wait([this](const boost::system::error_code& error)
        {
            if (!error)
            {
                next_day();
            }
        });
    }

private:
    void next_day()
    {
        auto now = system_clock::now();
        auto now_time_t = system_clock::to_time_t(now);
        std::tm now_tm = *std::localtime(&now_time_t);

        // Set the time to midnight today
        std::tm midnight_tm = now_tm;
        midnight_tm.tm_hour = 0;
        midnight_tm.tm_min = 0;
        midnight_tm.tm_sec = 0;

        // Convert the midnight time to time_point
        auto midnight_time_t = std::mktime(&midnight_tm);
        auto midnight_tp = system_clock::from_time_t(midnight_time_t);

        // If the current time is past midnight, set the timer for the next day
        if (now >= midnight_tp)
        {
            midnight_tp += hours(24);
        }

        auto duration_until_midnight = midnight_tp - now;

        // Set the timer to expire at the next midnight
        timer_.expires_after(duration_until_midnight);

        timer_.async_wait([this](const boost::system::error_code& error)
        {
            if (!error)
            {
                save_user_heartrate_data();
                next_day();
            }
        });
    }
    boost::asio::steady_timer timer_;
};


int main()
{
    try
    {
        aes_encrypt();

        int num_threads = std::thread::hardware_concurrency();

        handler_insert();

        net::io_context ioc{num_threads}; // Number of threads in the pool

        day_data_manager day_data_manager(ioc);

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
