#include "session.h"
#include "handler_utility.h"

session::session(tcp::socket socket)
: stream_(std::move(socket))
{
}

session::~session()
{
}

void session::start()
{
    do_read();
}

void session::do_read()
{
    auto self = shared_from_this();
    http::async_read(stream_, buffer_, request_,
        [self](beast::error_code ec, std::size_t)
        {
            if (!ec)
            {
                self->handle_request();
            }
        });
}

void session::handle_request()
{
    std::string path = std::string(request_.target());
    std::string::size_type query_pos = path.find('?');

    std::string path_to_uri;
    if (query_pos != std::string::npos)
    {
        // 쿼리 문자열이 포함된 경우 쿼리 문자열을 제외한 경로만 추출
        path_to_uri = path.substr(0, query_pos);
    }
    else
    {
        // 쿼리 문자열이 포함되지 않은 경우 전체 경로 사용
        path_to_uri = path;
    }
    auto handler = handlers.find(path_to_uri);
    if (handler != handlers.end())
    {
        handler->second(stream_, request_);
    }
    else
    {
        // No handler found for the path
        handle_not_found(stream_, request_);
    }

    // After handling request, start reading the next request
    do_read();
}
