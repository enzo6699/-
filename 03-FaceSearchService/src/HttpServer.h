#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include "PropertyConfigurator.h"
#include <log4cpp/Category.hh>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <memory>

class HttpServer
{
public:
	HttpServer();
	~HttpServer();

public:
    int StartHttpServer(int timeout);
	void StopHttpServer();
	void Run();

	/**
	Set a callback for a specified URI
	@param path the path for which to invoke the callback
	@param cb the callback function that gets invoked on requesting path
	@param cb_arg an additional context argument for the callback
	@return 0 on success, -1 if the callback existed already, -2 on failure
	*/
	int SetResource(const char *path, void *cb_arg);
private:
	static void http_handler(struct evhttp_request *req, void *arg);
    int registerService();

	struct evhttp *m_http_server;
	struct event_base *m_event_base;
	struct evhttp_bound_socket *m_handler;
    PropertyConfigurator &config;
    log4cpp::Category &logger;
};

#endif
