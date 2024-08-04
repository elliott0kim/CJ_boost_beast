#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <unordered_map>
#include <iostream>
#include <string>
#include <mutex>

inline std::mutex user_data_map_mutex; // for global variable user_data_map!

inline std::unordered_map<std::string, unsigned char*> user_data_map;

int set_user_data(const std::string& user_id, const unsigned char& data);

int get_user_data(const std::string& user_id, const std::string& heart_rate_date, unsigned char*& heart_rate_data_array);

int work_status_off_save_data(const std::string& user_id);

int get_last_heart_rate(std::unordered_map<std::string, unsigned char>& heart_rate_by_user_id);

#endif //DATA_COLLECTOR_H
