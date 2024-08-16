#include <vector>
#include "database_connector.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <iostream>
#include <stdexcept>
#include <charconv> // For std::from_chars
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "handler_utility.h"

// 하단의 시간조립 코드는 insert_heart_rate_by_scheduler 해당 함수가 12시 자정 넘어가면 저장하고 데이터 밀어주긴 하는데, 방어코드로 넣어두었다
int insert_heart_rate(const std::string& user_id, const unsigned char* user_data, const int total_count)
{
    try
    {
        // change unsigned char* to std::vector
        std::vector<unsigned char> blob_data;
        for (int idx = 0 ; idx < total_count ; idx++)
        {
            blob_data.push_back(user_data[idx]);
        }

        // 현재시각에서 현재 날짜의 00시 00분을 기준으로 몇분이나 지났는지 계산함
        // 만약, 배열의 길이보다 이게 길다?? 그러면은 이전 일 부터 근무 시작한것
        // 그럼 timestamp를 찢어서 저장해주어야 함
        // 현재시간 설정
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = *std::localtime(&now_time_t);

        // 00시 00분 정각 셋팅
        std::tm midnight = now_tm; // 현재 시각을 기준으로 구조체 초기화
        midnight.tm_hour = 0;
        midnight.tm_min = 0;
        midnight.tm_sec = 0;
        // target_tm.tm_mday = 3; // 3일로 설정 (필요시 수정)
        // target_tm.tm_mon = 7;  // 8월로 설정 (0부터 시작, 따라서 7 = 8월)
        // target_tm.tm_year = 2024 - 1900; // 2024년으로 설정 (1900년 기준)

        // std::tm을 std::chrono::system_clock::time_point로 변환
        auto midnight_time_t = std::mktime(&midnight);
        auto midnight_time_point = std::chrono::system_clock::from_time_t(midnight_time_t);

        // 두 시각의 차이 계산
        auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - midnight_time_point);
        int duration_minutes = static_cast<int>(duration.count());
        if (duration_minutes < 0)
        {
            return UNKNWON_ERROR;
        }
        if (duration_minutes < total_count)
        {
            // 어제자 오늘자 두개로 찢자..
            // 1 빼준 이유는 예를들어서, 00시 4분 - 00시 00분 = 4분 이지만
            // 갯수로는 00분꺼 1개, 01분꺼 2개, 02분꺼 3개, 03분꺼 4개, 04분꺼 5개 이다.
            std::vector<unsigned char> before_date_blob_data;
            // 1 더해준 이유는 00시 00분꺼도 1개로 치니까
            std::vector<unsigned char> current_date_blob_data;

            for (int before_date_index = 0 ; before_date_index < total_count - duration_minutes - 1 ; before_date_index++)
            {
                before_date_blob_data.push_back(user_data[before_date_index]);
            }

            for (int current_date_index = total_count - duration_minutes - 1 ; current_date_index < total_count ; current_date_index++)
            {
                current_date_blob_data.push_back(user_data[current_date_index]);
            }

            // 시간조립 씨발...
            std::ostringstream midnight_string_stream;
            midnight_string_stream << std::put_time(&midnight, "%Y-%m-%d %H:%M:%S");
            std::string before_date_blob = std::string(before_date_blob_data.begin(), before_date_blob_data.end());
            db_table.insert("workDate", "userId", "heartRateByteStream").values(midnight_string_stream.str(), user_id, before_date_blob).execute();

            // 양갈래니까.. 여기도 시간조립이요....
            midnight_time_t -= (total_count - duration_minutes) * SECOND_PER_MINUTE; // 1분 * 60초 -> 초로 환산해서 뺄셈하는 과정
            std::tm work_start_tm = *std::localtime(&midnight_time_t);
            std::ostringstream work_start_time_string_stream;
            work_start_time_string_stream << std::put_time(&work_start_tm, "%Y-%m-%d %H:%M:%S");
            std::string current_date_blob = std::string(current_date_blob_data.begin(), current_date_blob_data.end());
            db_table.insert("workDate", "userId", "heartRateByteStream").values(work_start_time_string_stream.str(), user_id, current_date_blob).execute();

            // 여기서 spring boot server에다가 request 두개 보내기
        }
        else
        {
            // 걍 빼서 저장~
            auto now_time = std::mktime(&now_tm);
            now_time -= (total_count) * SECOND_PER_MINUTE; // 1분 * 60초 -> 초로 환산해서 뺄셈하는 과정
            std::tm work_start_tm = *std::localtime(&now_time);
            std::ostringstream work_start_time_string_stream;
            work_start_time_string_stream << std::put_time(&work_start_tm, "%Y-%m-%d %H:%M:%S");
            std::string blob = std::string(blob_data.begin(), blob_data.end());
            db_table.insert("workDate", "userId", "heartRateByteStream").values(work_start_time_string_stream.str(), user_id, blob).execute();

            // 여기서 spring boot server에다가 request 하나 보내기
        }
    }
    catch (...)
    {
        return UNKNWON_ERROR;
    }
    return ERROR_NONE;
}

// 하단의 시간조립 코드는 insert_heart_rate_by_scheduler 해당 함수가 12시 자정 넘어가면 저장하고 데이터 밀어주긴 하는데, 방어코드로 넣어두었다
int select_heart_rate(const std::string& user_id, const std::string& heart_rate_date,
    unsigned char* heart_rate_data_array, bool* has_null, int* null_start_time_minute)
{
    try
    {
        *has_null = false;
        *null_start_time_minute = 0;

        // for debug & error handler variable
        int null_count = 0;
        // for debug & error handler variable endl

        // 데이터 선택
        std::string start_date_time = heart_rate_date;
        std::string end_date_time = heart_rate_date;
        start_date_time.append(" 00:00:00");
        end_date_time.append(" 23:59:59");
        mysqlx::RowResult res = db_table.select("DATE_FORMAT(workDate, '%Y-%m-%d %H:%i:%s') AS formattedDate", /*HEX(heartRateByteStream) AS */"heartRateByteStream", "userId")
             .where("userId = :user_id AND workDate BETWEEN :start_date AND :end_date")
             .bind("user_id", user_id)
             .bind("start_date", start_date_time)
             .bind("end_date", end_date_time)
             .execute();

        for (mysqlx::Row row : res)
        {
            // 가독성 구린 magic number index를 위한 주석처리
            // row[0] -> workDate column
            // row[1] -> heartRateByteStream column
            //
            // 왜 죄다 null 이거나 요일 등록 안댐? 에러처리. 사실 근데 이럴 경우는 없긴함~
            if (row.isNull() || row[0].isNull())
            {
                return UNKNWON_ERROR;
            }

            if (row[1].isNull())
            {
                std::string null_work_date_str = row[0].get<std::string>();
                std::tm null_work_date_date_time = {};
                std::istringstream null_work_date_string_stream(null_work_date_str);
                null_work_date_string_stream >> std::get_time(&null_work_date_date_time, "%Y-%m-%d %H:%M:%S");
                if (null_work_date_string_stream.fail())
                {
                    return UNKNWON_ERROR;
                }

                std::tm null_midnight = null_work_date_date_time; // 현재 시각을 기준으로 구조체 초기화
                null_midnight.tm_hour = 0;
                null_midnight.tm_min = 0;
                null_midnight.tm_sec = 0;

                auto null_work_date_date_time_t = std::mktime(&null_work_date_date_time);
                auto null_work_date_date_time_point = std::chrono::system_clock::from_time_t(null_work_date_date_time_t);
                auto null_midnight_time_t = std::mktime(&null_midnight);
                auto null_midnight_time_point = std::chrono::system_clock::from_time_t(null_midnight_time_t);

                // 두 시각의 차이 계산
                auto null_work_date = std::chrono::duration_cast<std::chrono::minutes>(null_work_date_date_time_point - null_midnight_time_point);
                int null_work_date_minute = static_cast<int>(null_work_date.count());

                if (null_work_date_minute < 0)
                {
                    return UNKNWON_ERROR;
                }

                *has_null = true;
                *null_start_time_minute = null_work_date_minute;

                // for debug & error handler variable
                null_count += 1;
                if (null_count == 2)
                {
                    return UNKNWON_ERROR;
                }
                // for debug & error handler variable endl
            }
            else
            {
                // 시간조립.. 화가난다 정말
                // 우선 DB에서 불러온 시간 면저 뺄셈을 위한 시간 타입으로 변환

                std::string work_date_str = row[0].get<std::string>();
                std::string test = row[2].get<std::string>();
                std::tm work_date_date_time = {};
                std::istringstream work_date_string_stream(work_date_str);
                work_date_string_stream >> std::get_time(&work_date_date_time, "%Y-%m-%d %H:%M:%S");
                if (work_date_string_stream.fail())
                {
                    return UNKNWON_ERROR;
                }

                // 밤 12시 정각 계산해야지 어디가~
                // 00시 00분 정각 셋팅
                std::tm midnight = work_date_date_time; // 현재 시각을 기준으로 구조체 초기화
                midnight.tm_hour = 0;
                midnight.tm_min = 0;
                midnight.tm_sec = 0;

                // 그리고 DB에 등록된 시각 - midnight 을 계산하자
                // std::tm을 std::chrono::system_clock::time_point로 변환
                auto work_date_date_time_t = std::mktime(&work_date_date_time);
                auto work_date_date_time_point = std::chrono::system_clock::from_time_t(work_date_date_time_t);
                auto midnight_time_t = std::mktime(&midnight);
                auto midnight_time_point = std::chrono::system_clock::from_time_t(midnight_time_t);

                // 두 시각의 차이 계산
                auto term = std::chrono::duration_cast<std::chrono::minutes>(work_date_date_time_point - midnight_time_point);
                int term_minute = static_cast<int>(term.count());
                if (term_minute < 0)
                {
                    return UNKNWON_ERROR;
                }
                // BLOB 데이터 추출해서 벡터변환
                std::string blob_data = row[1].get<std::string>();
                std::vector<unsigned char> blob_data_vector(blob_data.begin(), blob_data.end());

                for (int idx = term_minute ; idx < term_minute + blob_data_vector.size() ; idx++)
                {
                    heart_rate_data_array[idx] = blob_data_vector.at(idx - term_minute);
                }
            }
        }
        return ERROR_NONE;
    }
    catch (...)
    {
        return UNKNWON_ERROR;
    }
}

int insert_heart_rate_by_scheduler(const std::string& user_id, const unsigned char* user_data, const int total_count)
{
    long long real_duration_error = ERROR_DURATION_INITIALIZER;
    std::ostringstream midnight_string_stream;
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    // 얘는 그냥 00으로 지운애임
    std::tm midnight_tm = now_tm;
    midnight_tm.tm_hour = 0;
    midnight_tm.tm_min = 0;
    midnight_tm.tm_sec = 0;

    auto midnight_today = std::chrono::system_clock::from_time_t(std::mktime(&midnight_tm));
    auto midnight_tomorrow = midnight_today + std::chrono::hours(24);

    auto duration_to_midnight_today = duration_cast<std::chrono::seconds>(midnight_today - now).count();
    auto duration_to_midnight_tomorrow = duration_cast<std::chrono::seconds>(midnight_tomorrow - now).count();


    if (std::abs(duration_to_midnight_today) > std::abs(duration_to_midnight_tomorrow))
    {
        real_duration_error = duration_to_midnight_tomorrow;
        // midnight_tomorrow 얘 갖다 써야함
        std::time_t midnight_tomorrow_time_t = std::chrono::system_clock::to_time_t(midnight_tomorrow);
        std::tm midnight_tomorrow_tm = *std::localtime(&midnight_tomorrow_time_t);

        midnight_string_stream << std::put_time(&midnight_tomorrow_tm, "%Y-%m-%d %H:%M:%S");
    }
    else if (std::abs(duration_to_midnight_today) < std::abs(duration_to_midnight_tomorrow))
    {
        real_duration_error = duration_to_midnight_today;
        // midnight_today 얘 갖다 써야함
        midnight_string_stream << std::put_time(&midnight_tm, "%Y-%m-%d %H:%M:%S");
    }
    else
    {
        // 둘의 차이가 동일한건.. 딱 12시간 차이날 때 인데 이럴경우는 없음!
        return UNKNWON_ERROR;
    }

    // 차이가 2분 미만이면은 그냥 저장해주기
    if (std::abs(real_duration_error) < 121)
    {
        std::vector<unsigned char> blob_data;
        for (int idx = 0 ; idx < total_count ; idx++)
        {
            blob_data.push_back(user_data[idx]);
        }
        std::string blob = std::string(blob_data.begin(), blob_data.end());
        db_table.insert("workDate", "userId", "heartRateByteStream").values(midnight_string_stream.str(), user_id, blob).execute();
    }
    else
    {
        return OVER_DURATION_ERROR;
    }
    return ERROR_NONE;
}

int select_add_data(std::vector<std::string>& name, std::vector<int>& heart_rate, std::vector<int>& walk, std::vector<float>& move)
{
    try
    {
        mysqlx::RowResult res = db_table_two.select("name", "heart_rate", "walk", "move").execute();
        int i = 0;
        for (mysqlx::Row row : res)
        {
            PRINT(i++);

            if (row.isNull())
            {
                return UNKNWON_ERROR;
            }

            const std::vector<unsigned char> iv = generate_random_iv(IV_SIZE);

            // BLOB 데이터 추출해서 벡터변환
            std::string current_name = row[0].get<std::string>();

            // 디코딩
            std::string blob_heart_rate = row[1].get<std::string>();
            //std::vector<unsigned char> blob_data_vector(blob_heart_rate.begin(), blob_heart_rate.end());
            std::vector<unsigned char> blob_data_vector = base64_decode(blob_heart_rate);

            std::string blob_heart_rate_decrypt = aes_decrypt(blob_data_vector, key, iv);
            int heart_rate_data;
            auto [ptr, ec] = std::from_chars(blob_heart_rate_decrypt.data(), blob_heart_rate_decrypt.data() + blob_heart_rate_decrypt.size(), heart_rate_data);

            std::vector<int> blob_data_decrypt_vector(blob_heart_rate_decrypt.begin(), blob_heart_rate_decrypt.end());

            int walk_data = row[2].get<int>();
            float move_data = row[3].get<float>();

            name.push_back(current_name);

            heart_rate.push_back(heart_rate_data);

            walk.push_back(walk_data);

            move.push_back(move_data);
        }
        return ERROR_NONE;
    }
    catch (...)
    {
        return UNKNWON_ERROR;
    }

}

std::vector<unsigned char> generate_random_iv(size_t iv_size)
{
    std::vector<unsigned char> iv(iv_size);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        throw std::runtime_error("Failed to generate random IV");
    }
    return iv;
}

void aes_encrypt()
{
    mysqlx::RowResult res = db_table_two.select("id",  "heart_rate").execute();
    PRINT(1);
    std::unordered_map<int, unsigned> temp_map;
    for (mysqlx::Row row : res)
    {
        if (row.isNull())
        {
            return;
        }

        int id = row[0].get<int>();
        int heart_rate_int = row[1].get<int>();
        unsigned char heart_rate = (unsigned char)heart_rate_int;
        PRINT(2);
        temp_map[id] = heart_rate;
    }

    PRINT(3);
    const std::vector<unsigned char> iv = generate_random_iv(IV_SIZE);
    PRINT(4);

    for (const auto& pair : temp_map)
    {
        int id = pair.first;
        unsigned char heart_rate_data = pair.second;
        PRINT(5);
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        std::vector<unsigned char> ciphertext(1 + AES_BLOCK_SIZE);
        PRINT(6);
        int len;
        int ciphertext_len;
        PRINT(7);
        EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data());
        EVP_EncryptUpdate(ctx, ciphertext.data(), &len, &heart_rate_data, 1);
        ciphertext_len = len;
        PRINT(8);
        EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
        ciphertext_len += len;
        PRINT(9);
        ciphertext.resize(ciphertext_len);
        EVP_CIPHER_CTX_free(ctx);
        PRINT(10);

        std::string blob = base64_encode(ciphertext);
        //std::string blob = std::string(ciphertext.begin(), ciphertext.end());

        db_table_two.update().set("heart_rate_blob", blob).where("id = :id").bind("id", id).execute();
    }
}

std::string aes_decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> plaintext(ciphertext.size());
    int len;
    int plaintext_len;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data());
    EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size());
    plaintext_len = len;

    EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
    plaintext_len += len;

    plaintext.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);

    return std::string(plaintext.begin(), plaintext.end());
}



std::string base64_encode(const std::vector<unsigned char>& ciphertext)
{
    BIO* bio;
    BIO* b64;
    BUF_MEM* bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, ciphertext.data(), ciphertext.size());
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encodedData(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return encodedData;
}

std::vector<unsigned char> base64_decode(const std::string& encoded_string)
{
    static const std::unordered_map<char, int> base64_index = [] {
        std::unordered_map<char, int> map;
        for (int i = 0; i < 64; ++i) {
            map[base64_chars[i]] = i;
        }
        return map;
    }();

    std::vector<unsigned char> decoded_data;
    int val = 0, valb = -8;

    for (char c : encoded_string) {
        if (base64_index.find(c) != base64_index.end()) {
            val = (val << 6) + base64_index.at(c);
            valb += 6;
            if (valb >= 0) {
                decoded_data.push_back(static_cast<unsigned char>((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
    }

    return decoded_data;
}
