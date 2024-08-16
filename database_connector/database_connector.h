#ifndef DATABASE_CONNECTOR_H
#define DATABASE_CONNECTOR_H
#include <string>
#include <mysqlx/xdevapi.h>
#include <unordered_map>
#include "define_utility.h"

inline mysqlx::Session db_session(LOCAL_HOST, DATABASE_PORT_NUM, DATABASE_ID, DATABASE_PW);

inline mysqlx::Schema db_schema = db_session.getSchema(DATABASE_NAME);

inline mysqlx::Table db_table = db_schema.getTable(DATABASE_TABLE);

inline mysqlx::Table db_table_two = db_schema.getTable("CJ_DUMMY");

int insert_heart_rate(const std::string& user_id, const unsigned char* user_data, const int total_count);

int select_heart_rate(const std::string& user_id, const std::string& heart_rate_date,
    unsigned char* heart_rate_data_array, bool* has_null, int* null_start_time_minute);

int insert_heart_rate_by_scheduler(const std::string& user_id, const unsigned char* user_data, const int total_count);

int select_add_data(std::vector<std::string>& name, std::vector<int>& heart_rate, std::vector<int>& walk, std::vector<float>& move);

void aes_encrypt();

std::string base64_encode(const std::vector<unsigned char>& ciphertext);

std::vector<unsigned char> base64_decode(const std::string& encoded_string);

std::string aes_decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv);

std::vector<unsigned char> generate_random_iv(size_t iv_size);

inline std::vector<unsigned char> key = {'Z', 'G', '9', 'u', 'Z', '2', 'h', '5', 'd', 'W', '4', 'g', 'e', 'p', 'h', 'b', 'm', 'c', 'g', 'e', 'n', 'p', 'h', 'b', 'm', 'c', '='};



inline const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

#endif //DATABASE_CONNECTOR_H
