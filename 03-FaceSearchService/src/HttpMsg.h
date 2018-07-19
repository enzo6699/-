#ifndef _HTTP_MSG_H_
#define _HTTP_MSG_H_

#include <iostream>
#include <stdlib.h>
#include <map>

typedef struct _http_msg_t {
    _http_msg_t()
    {
        request_uri = NULL;
        request_uri_len = 0;

        request_data.empty();

        respone_data.empty();
    }
    virtual ~_http_msg_t()
    {
        if (request_uri != NULL)
        {
            free(request_uri);
            request_uri = NULL;
        }
    }

    struct evhttp_request *req;
    char *request_uri;
    int request_uri_len;

    std::string request_data;
    std::string respone_data;
} http_msg_t;

#endif
