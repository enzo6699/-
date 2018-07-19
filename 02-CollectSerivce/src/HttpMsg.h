#ifndef _HTTP_MSG_H_
#define _HTTP_MSG_H_

#include <iostream>
#include <stdlib.h>
#include <map>

typedef struct _http_msg_t {
    struct evhttp_request *req = NULL;
    std::string request_uri;

    std::string request_data;
    std::string respone_data;
} http_msg_t;

#endif
