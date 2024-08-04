#include <rapidjson/document.h>
#include <thread>
#include <limits>
#include "handler_uri.h"
#include "token_parser.h"
#include "data_collector.h"
#include "define_utility.h"
#include "make_json_format.h"
#include "message_to_define.h"

#ifndef USER_DATA_ARRAY_MAX_COUNT
#define USER_DATA_ARRAY_MAX_COUNT                               HOUR_16
#endif // USER_DATA_ARRAY_MAX_COUNT

#ifndef USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT
#define USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT                    HOUR_24
#endif // USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT

void handle_get_heart_rate(beast::tcp_stream& stream, http::request<http::string_body>& req)
{
    std::string target_string = req.target();
    if (req.method() != http::verb::get)
    {
        handle_not_found(stream, req);
        return;
    }

    std::string::size_type query_pos = target_string.find('?');

    std::string query_string;
    if (query_pos != std::string::npos)
    {
        query_string = target_string.substr(query_pos + 1);
    }

//userId=elliott&heartRateDate=2024-06-04    ->    uri query example
    std::unordered_map<std::string, std::string> query_params;
    std::stringstream query_stream(query_string);
    std::string key_value;

    while (std::getline(query_stream, key_value, '&'))
    {
        std::string::size_type pos = key_value.find('=');
        if (pos != std::string::npos)
        {
            std::string key = key_value.substr(0, pos);
            std::string value = key_value.substr(pos + 1);
            query_params[key] = value;
        }
    }

    // user_id , heart_rate_date 두개만 받아야 되니까!
    // magic number 죄송합니다~
    if (query_params.size() != 2)
    {
        parsing_error(stream, req);
        return;
    }

    for (const auto& pair : query_params)
    {
        if (pair.first != "userId" && pair.first != "heartRateDate")
        {
            parsing_error(stream, req);
            return;
        }
    }

    unsigned char* response_heart_rate_array;
    response_heart_rate_array = nullptr;
    int ret = get_user_data(query_params["userId"], query_params["heartRateDate"], response_heart_rate_array);
    if (ret != ERROR_NONE)
    {
        if (response_heart_rate_array != nullptr)
        {
            delete response_heart_rate_array;
        }
        unknown_error(stream, req);
        return;
    }

    // response_heart_rate_array for문돌려서 response 조립하고, delete꼭해줘야함
    // 저짝 get_user_data 함수에서 new한거임.. 내가 짠거지만 코드 뭣같이 짯네... 여기서 malloc 해줘야지 왜 그랫니
    //
    // 하단은 get_user_data 함수에서 new 해준 코드
    // heart_rate_data_array = new unsigned char[USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT];
    std::string response_heart_rate_string;
    response_heart_rate_string.append("[");
    for (int idx = 0 ; idx < USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT ; idx++)
    {
        std::string heart_rate_string = std::to_string(static_cast<int>(response_heart_rate_array[idx]));
        response_heart_rate_string.append(heart_rate_string);
        response_heart_rate_string.append(", ");
    }
    response_heart_rate_string.pop_back();  // ", " 문자 제거
    response_heart_rate_string.pop_back();  // ", " 문자 제거
    response_heart_rate_string.append("]");
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::content_type, "application/json");
    res.body() = make_json_format(SUCCESS, response_heart_rate_string);
    res.prepare_payload();
    http::write(stream, res);

    delete response_heart_rate_array;
}

void handle_set_heart_rate(beast::tcp_stream& stream, http::request<http::string_body>& req)
{
    std::string user_id = token_parser::parse_iss(req);
    //PRINT(iss);
    if (req.method() != http::verb::post)
    {
        handle_not_found(stream, req);
        return;
    }

    if (req.body().empty())
    {
        parsing_error(stream, req);
        return;
    }

    rapidjson::Document document;
    document.Parse(req.body().c_str());
    if (document.HasParseError())
    {
        parsing_error(stream, req);
        return;
    }

    if (document.HasMember("data") && document["data"].IsInt())
    {
        int data = document["data"].GetInt();
        if (data > 0 && data <= std::numeric_limits<unsigned char>::max())
        {
            unsigned char unsigned_data = static_cast<unsigned char>(data);
            int ret = set_user_data(user_id, unsigned_data);
            if (ret != ERROR_NONE)
            {
                unknown_error(stream, req);
                return;
            }

            // 정상적으로 저장 되었을 경우, 어떻게 처리할지 생각해!
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.body() = make_json_format(SUCCESS, "");
            res.prepare_payload();
            http::write(stream, res);
        }
        else
        {
            parsing_error(stream, req);
            return;
        }

    }
    else
    {
        parsing_error(stream, req);
        return;
    }
}

// spring boot response
void handle_work_status_off(beast::tcp_stream& stream, http::request<http::string_body>& req)
{
    if (req.method() != http::verb::post)
    {
        handle_not_found(stream, req);
        return;
    }

    if (req.body().empty())
    {
        parsing_error(stream, req);
        return;
    }

    rapidjson::Document document;
    document.Parse(req.body().c_str());
    if (document.HasParseError())
    {
        parsing_error(stream, req);
        return;
    }

    if (document.HasMember("userId") && document["userId"].IsString())
    {
        std::string user_id = document["userId"].GetString();
        int ret = work_status_off_save_data(user_id);
        if (ret != ERROR_NONE)
        {
            unknown_error(stream, req);
            return;
        }
        else
        {
            // 정상적으로 저장 되었을 경우, 어떻게 처리할지 생각해!
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.body() = make_json_format(SUCCESS, "");
            res.prepare_payload();
            http::write(stream, res);
        }
    }
    else
    {
        parsing_error(stream, req);
        return;
    }
}

void handle_last_heart_rate(beast::tcp_stream& stream, http::request<http::string_body>& req)
{
    std::string target_string = req.target();
    if (req.method() != http::verb::get)
    {
        handle_not_found(stream, req);
        return;
    }

    std::string::size_type query_pos = target_string.find('?');

    std::string query_string;
    if (query_pos != std::string::npos)
    {
        query_string = target_string.substr(query_pos + 1);
    }

    // userId=elliott,jeremy    ->    uri query example
    std::unordered_map<std::string, unsigned char> heart_rate_by_user_id;

    size_t pos = query_string.find('=');
    if (pos != std::string::npos)
    {
        std::string values = query_string.substr(pos + 1); // '=' 다음의 부분 추출

        // 쉼표를 기준으로 문자열을 나누기
        std::string token;
        std::stringstream ss(values);

        while (std::getline(ss, token, ','))
        {
            heart_rate_by_user_id[token] = 0;
        }
    }

    if (heart_rate_by_user_id.size() == 0)
    {
        parsing_error(stream, req);
        return;
    }

    int ret = get_last_heart_rate(heart_rate_by_user_id);
    if (ret != ERROR_NONE)
    {
        unknown_error(stream, req);
        return;
    }

    std::string response_heart_rate_by_user_id;
    response_heart_rate_by_user_id.append("{");
    for (const auto& pair : heart_rate_by_user_id)
    {
        std::string user_id = pair.first;
        unsigned char last_heart_rate = pair.second;
        response_heart_rate_by_user_id.append("\"");
        response_heart_rate_by_user_id.append(user_id);
        response_heart_rate_by_user_id.append("\"");
        response_heart_rate_by_user_id.append(" : ");

        std::string last_heart_rate_string = std::to_string(static_cast<int>(last_heart_rate));
        response_heart_rate_by_user_id.append(last_heart_rate_string);
        response_heart_rate_by_user_id.append(", ");
    }
    response_heart_rate_by_user_id.pop_back();  // ", " 문자 제거
    response_heart_rate_by_user_id.pop_back();  // ", " 문자 제거
    response_heart_rate_by_user_id.append("}");

    // 정상적으로 저장 되었을 경우, 어떻게 처리할지 생각해!
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::content_type, "application/json");
    // 여기서 데이터 넣어줘야함
    res.body() = make_json_format(SUCCESS, response_heart_rate_by_user_id);
    res.prepare_payload();
    http::write(stream, res);
}


void handler_insert()
{
    handlers.insert({"/getHeartRate", handle_get_heart_rate});
    handlers.insert({"/setHeartRate", handle_set_heart_rate});
    handlers.insert({"/workStatusOff", handle_work_status_off});
    handlers.insert({"/lastHeartRate", handle_last_heart_rate});
}
 