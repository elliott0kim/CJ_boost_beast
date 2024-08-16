#ifndef MAKE_JSON_FORMAT_H
#define MAKE_JSON_FORMAT_H

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

inline string make_json_format (const string& message, const string& data)
{
    stringstream cj_format;
    cj_format << "{\n"
        << "  \"message\" : \"" << message << "\",\n"
        << "  \"data\" : " << data << "\n"
        << "}";
    return cj_format.str();
}


#endif //MAKE_JSON_FORMAT_H
