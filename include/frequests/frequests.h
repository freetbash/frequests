/**
 * @file frequests.h
 * @author Freet-Bash (3141495167@qq.com)
 * @brief  HTTP 请求网络库
 * @date 2022-09-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef FREQUESTS
#define FREQUESTS

#define HTTP_DELIM "\r\n"
#define HTTP_DELIM_DOUBLE "\r\n\r\n"
#define HTTP_VERSION "HTTP/1.1"
#define F_USERAGENT "Frequests/0.01"
#define MAX_SIZE 1024

#include <vector>
#include <string>
#include <map>
#include <xsystem/xsystem.h>

class Frequest;
class Fresponse;

enum HTTP_METHOD{
    GET,
    POST,
    HEAD,
    PUT,
    DEL
};

class Frequest{
    private:
        // net fd
        xsystem::net::FSocket fsocket;
        std::string dst_ip;
        int dst_port;        std::map<std::string, std::string> headers;
        char *body;
        unsigned long body_len;
        std::string construct_http_header(HTTP_METHOD method,std::string url);
        void handle_http_response(Fresponse *response);
        void paser_http_response_header(Fresponse *response,std::string http_header);
    public:
        std::string url;
        Frequest *const set_header(std::string key,std::string value);
        void set_proxy(std::string proxy);
        void add_body(char *data,unsigned long len);
        void clear();

        Fresponse get(std::string url);
        Fresponse del(std::string url);
        Fresponse put(std::string url);
        Fresponse head(std::string url);
        Fresponse post(std::string url);

        Frequest();
        ~Frequest();

};

class Fresponse{
    public:
        Frequest *request;
        std::string url;
        int status_code;    // 200
        std::string reson;  // Ok
        std::map<std::string,std::string> headers;
        std::string text;   // 
        char *content;
        unsigned long content_len;
        Fresponse();
        ~Fresponse();
        
};


#endif