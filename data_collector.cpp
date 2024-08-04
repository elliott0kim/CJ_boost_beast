#include "data_collector.h"
#include "define_utility.h"
#include "database_connector.h"

#ifndef USER_DATA_ARRAY_MAX_COUNT
#define USER_DATA_ARRAY_MAX_COUNT                               HOUR_16
#endif // USER_DATA_ARRAY_MAX_COUNT

#ifndef USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT
#define USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT                    HOUR_24
#endif // USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT

int set_user_data(const std::string& user_id, const unsigned char& data)
{
    // 뮤텍스로 안잠그면 개박살남... 알지?
    std::lock_guard<std::mutex> lock(user_data_map_mutex); // 뮤텍스 잠금

    auto it = user_data_map.find(user_id);
    if (it != user_data_map.end())
    {
        try
        {
            // 960개 넘어가면 bytestream으로 말아서 저장하는 루틴 필요
            int total_count = 0;
            for (int idx = 0 ; idx < USER_DATA_ARRAY_MAX_COUNT ; idx++)
            {
                if (user_data_map[user_id][idx] != UNSIGNED_CHAR_MIN)
                {
                    total_count += 1;
                }
                else
                {
                    break;
                }
            }
            PRINT(total_count);
            if (total_count == USER_DATA_ARRAY_MAX_COUNT)
            {
                // 퇴근 안찍은것으로 간주
                // DB에 밀어넣고 그냥 퇴근으로 바꿔주기
                // 그리고 return 으로 끊고 나가야지?
                // 그리고 spring boot 서버에다가 알려주기
                int ret = insert_heart_rate(user_id, user_data_map[user_id], total_count);
                delete user_data_map[user_id];
                user_data_map[user_id] = nullptr;
                user_data_map.erase(user_id);
                if (ret == ERROR_NONE)
                {
                    return ERROR_NONE;
                }
                else
                {
                    return UNKNWON_ERROR;
                }
            }

            user_data_map[user_id][total_count] = data;
        }
        catch (...)
        {
            return UNKNWON_ERROR;
        }
    }
    else
    {
        try
        {
            unsigned char* user_data_array = new unsigned char[USER_DATA_ARRAY_MAX_COUNT];
            memset(user_data_array, UNSIGNED_CHAR_MIN, USER_DATA_ARRAY_MAX_COUNT);
            user_data_array[0] = data;
            user_data_map[user_id] = user_data_array;
        }
        catch (...)
        {
            return UNKNWON_ERROR;
        }
    }
    return ERROR_NONE;
}

int get_user_data(const std::string& user_id, const std::string& heart_rate_date, unsigned char*& heart_rate_data_array)
{
    try
    {
        // 뮤텍스로 안잠그면 개박살남... 알지?
        std::lock_guard<std::mutex> lock(user_data_map_mutex); // 뮤텍스 잠금

        heart_rate_data_array = new unsigned char[USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT];
        memset(heart_rate_data_array, UNSIGNED_CHAR_MIN, USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT);

        bool has_null = false;
        int null_start_time_minute = 0;

        memset(heart_rate_data_array, UNSIGNED_CHAR_MIN, USER_DATA_ARRAY_FOR_RETURN_MAX_COUNT);
        int ret = select_heart_rate(user_id, heart_rate_date, heart_rate_data_array, &has_null, &null_start_time_minute);
        if  (ret != ERROR_NONE)
        {
            delete heart_rate_data_array;
            heart_rate_data_array = nullptr;
            return UNKNWON_ERROR;
        }

        if (has_null == true)
        {
            // 아 DB에 없으세요? 그럼 지금꺼 넣어주기
            auto it = user_data_map.find(user_id);
            if (it != user_data_map.end())
            {
                int user_data_count = 0;
                for (int idx = 0; idx < USER_DATA_ARRAY_MAX_COUNT ; idx++)
                {
                    if (user_data_map[user_id][idx] != 0)
                    {
                        user_data_count++;
                    }
                    else
                    {
                        break;
                    }
                }

                for (int idx = null_start_time_minute ; idx < null_start_time_minute + user_data_count ; idx++)
                {
                    heart_rate_data_array[idx] = user_data_map[user_id][idx - null_start_time_minute];
                }
            }
            // 엥 DB에도 없는데 내가 쥐고있는것도 없어? 졸라이상해~
            else
            {
                delete heart_rate_data_array;
                heart_rate_data_array = nullptr;
                return UNKNWON_ERROR;
            }
        }
        return ERROR_NONE;
    }
    catch (...)
    {
        delete heart_rate_data_array;
        heart_rate_data_array = nullptr;
        return UNKNWON_ERROR;
    }
}

int work_status_off_save_data(const std::string& user_id)
{
    try
    {
        // 뮤텍스로 안잠그면 개박살남... 알지?
        std::lock_guard<std::mutex> lock(user_data_map_mutex); // 뮤텍스 잠금
        auto it = user_data_map.find(user_id);
        if (it != user_data_map.end())
        {
            // 전체 갯수만 우선 세주고
            int user_data_count = 0;
            for (int idx = 0; idx < USER_DATA_ARRAY_MAX_COUNT ; idx++)
            {
                if (user_data_map[user_id][idx] != 0)
                {
                    user_data_count++;
                }
                else
                {
                    break;
                }
            }
            if (user_data_count != 0)
            {
                // 그냥 슥 저장요~
                int ret = insert_heart_rate(user_id, user_data_map[user_id], user_data_count);
                if (ret != ERROR_NONE)
                {
                    return UNKNWON_ERROR;
                }
                // 에러가 나도 일단 지우자.. 서버 터지는것보다 우선 지우는게 남
                // 하루치 데이터 날라가도 어쩔수없다~
                delete user_data_map[user_id];
                user_data_map[user_id] = nullptr;
                user_data_map.erase(user_id);
            }
            else
            {
                return UNKNWON_ERROR;
            }
        }
        else
        {
            return UNKNWON_ERROR;
        }
        return ERROR_NONE;
    }
    catch (...)
    {
        return UNKNWON_ERROR;
    }
}

int get_last_heart_rate(std::unordered_map<std::string, unsigned char>& heart_rate_by_user_id)
{
    try
    {
        // 뮤텍스로 안잠그면 개박살남... 알지?
        std::lock_guard<std::mutex> lock(user_data_map_mutex); // 뮤텍스 잠금

        for (const auto& pair : heart_rate_by_user_id)
        {
            std::string user_id = pair.first;
            auto it = user_data_map.find(user_id);
            if (it != user_data_map.end())
            {
                for (int heart_rate_idx = 0 ; heart_rate_idx < USER_DATA_ARRAY_MAX_COUNT ; heart_rate_idx++)
                {
                    if (user_data_map[user_id][heart_rate_idx] == 0)
                    {
                        // 마지막 데이터를 여기서 리턴해줘야 함.
                        // 직전 인덱스가 마지막 heart rate 이다!
                        // 그런데 혹시라도 이제막 근무를 시작했다거나...
                        // 아니면은 서버의 문제상 값을 기록하고있지 못했을경우를 대비하여 error 처리
                        if (heart_rate_idx != 0)
                        {
                            heart_rate_by_user_id[user_id] = user_data_map[user_id][heart_rate_idx - 1];
                            break;
                        }
                        else
                        {
                            // 못찾아도 일단 pass....
                            // 다른애들꺼라도 돌려줘야대니까....
                            // 근데 문제는 있다...
                            // 이거 에러로그라도 찍어줘야하는데
                            // 로그레벨 설정해서 로그찍는 클래스나 function이라도 만들어야하는데
                            // 그럴 시간이 없어지금... 나혼자 이거 하는것도 개빡세고 힘들어... 죽겠어.....
                            // 그정도까지 하면 이건 회사에서 돈받고해야지 공모전 따위에 그런것까지 다 해야하나?
                            ;
                        }
                    }
                }
            }
            else
            {
                // 못찾아도 일단 pass....
                // 다른애들꺼라도 돌려줘야대니까....
                // 근데 문제는 있다...
                // 이거 에러로그라도 찍어줘야하는데
                // 로그레벨 설정해서 로그찍는 클래스나 function이라도 만들어야하는데
                // 그럴 시간이 없어지금... 나혼자 이거 하는것도 개빡세고 힘들어... 죽겠어.....
                ;
            }
        }
        return ERROR_NONE;
    }
    catch (...)
    {
        return UNKNWON_ERROR;
    }
}
