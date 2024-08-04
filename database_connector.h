#ifndef DATABASE_CONNECTOR_H
#define DATABASE_CONNECTOR_H
#include <string>
#include <mysqlx/xdevapi.h>
#include "define_utility.h"

inline mysqlx::Session db_session(LOCAL_HOST, DATABASE_PORT_NUM, DATABASE_ID, DATABASE_PW);

inline mysqlx::Schema db_schema = db_session.getSchema(DATABASE_NAME);

inline mysqlx::Table db_table = db_schema.getTable(DATABASE_TABLE);

int insert_heart_rate(const std::string& user_id, const unsigned char* user_data, const int total_count);

int select_heart_rate(const std::string& user_id, const std::string& heart_rate_date,
    unsigned char* heart_rate_data_array, bool* has_null, int* null_start_time_minute);

#endif //DATABASE_CONNECTOR_H
