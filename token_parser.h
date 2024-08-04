#ifndef TOKEN_PARSER_H
#define TOKEN_PARSER_H

#include <string>
#include "boost_header.h"

using namespace std;

class token_parser
{
public:
    token_parser();
    ~token_parser();
    static string parse_iss(http::request<http::string_body>& req);
};

#endif //TOKEN_PARSER_H
