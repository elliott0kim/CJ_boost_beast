#include "jwt-cpp/jwt.h"
#include "token_parser.h"
#include "define_utility.h"
using namespace std;

token_parser::token_parser()
{
}

token_parser::~token_parser()
{
}

string token_parser::parse_iss(http::request<http::string_body>& req)
{
    try
    {
        string token = "";
        if (req.find(http::field::authorization) != req.end()) {
            token = string(req[http::field::authorization]);
            // "Bearer " 제거
            const std::string bearer = "Bearer ";
            if (token.substr(0, bearer.size()) == bearer) {
                token = token.substr(bearer.size());
            }
        }

        auto decoded = jwt::decode(token);
        string claims = decoded.get_payload_claim("iss").as_string();
        return claims;
    }
    catch (const exception& e)
    {
        std::cerr << "Error decoding token: " << e.what() << std::endl;
        return "Error";
    }

}
