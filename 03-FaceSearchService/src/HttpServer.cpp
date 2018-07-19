#include "HttpServer.h"
#include <string.h>
#include "utils/StringUtil.h"
#include "evhttp.h"
#include "HttpMsg.h"
#include "common/Logger.h"
#include "BaseMsgHandler.h"
#include "utils/ConvertCharCode.h"

#include <sys/types.h>

#if defined(__GNUC__)
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

HttpServer::HttpServer()
: m_event_base(nullptr)
, m_http_server(nullptr)
, m_handler(nullptr)
, config(PropertyConfigurator::Instance())
, logger(log4cpp::Category::getRoot())
{
}

HttpServer::~HttpServer()
{
	if (m_http_server != nullptr)
	{
		evhttp_free(m_http_server);
		m_http_server = nullptr;
	}
}

int HttpServer::StartHttpServer(int timeout)
{
	int ret = 0;
#if defined(_MSC_VER)
	WSADATA wsaData;
	unsigned short versionRequested = MAKEWORD(1, 1);
	ret = WSAStartup(versionRequested, &wsaData);
	if (ret != 0)
	{
		log4cpp::Category::getRoot().error("WSAStartup failed! ret:%d", ret);
		return 1;
	}
#endif
	if (m_event_base == nullptr)
	{
		m_event_base = event_init();
	}

	m_http_server = evhttp_new(m_event_base);
	if (m_http_server == nullptr)
	{
        log4cpp::Category::getRoot().error("evhttp_new failed");
		return 3;
	}

    const std::string address("0.0.0.0");
    if (evhttp_bind_socket(m_http_server, address.c_str(), config.port) == 0) {
        log4cpp::Category::getRoot().info("Succeeded listen on %s:%d", address.c_str(), config.port);
    } else {
        log4cpp::Category::getRoot().error("Failed binding port");
        return 4;
    }

	// 设置请求超时时间
    timeout = timeout <= 0 ? 8 : timeout;
	evhttp_set_timeout(m_http_server, timeout);

	return 0;
}

void HttpServer::StopHttpServer()
{
	if (m_http_server != nullptr)
	{
		event_loopbreak();
		evhttp_free(m_http_server);
		m_http_server = nullptr;
	}
}

void HttpServer::Run()
{
    if (registerService() != 0) {
        logger.warn("Failed register service");
    }

	if (m_http_server == nullptr)
	{
		log4cpp::Category::getRoot().error("evhttp_start failed");
	}
	else
	{
		if (m_event_base != NULL)
			event_base_dispatch(m_event_base);
		//event_dispatch();
	}
}

int HttpServer::SetResource(const char *path, void *cb_arg)
{
	return evhttp_set_cb(m_http_server, path,
		http_handler, cb_arg);
}

void HttpServer::http_handler(struct evhttp_request *req, void *arg)
{
	BaseMsgHandler *pHandler = (BaseMsgHandler *)arg;
	if (pHandler == nullptr)
	{
		log4cpp::Category::getRoot().error("pHandler is null!");
		return;
	}

	// 分析请求
	http_msg_t msg;
	msg.req = req;
	msg.request_uri = strdup((char*)evhttp_request_uri(req));

    evbuffer *buf = evhttp_request_get_input_buffer(req);
    size_t length = EVBUFFER_LENGTH(buf);
    unsigned char *data= EVBUFFER_DATA(buf);
    msg.request_data.append(data, data + length);

	//msg.request_data = std::string((char*)EVBUFFER_DATA(req->input_buffer), buffer_data_len);
//    tce::ConvertCharCode::Utf8ToAnsi(std::string((const char*)input_buffer, buffer_data_len), msg.request_data);

//    log4cpp::Category::getRoot().debug("request_data:%s; buffer_data_len:%d", msg.request_data.c_str(), length);
	pHandler->dealMsg(&msg);

	// 返回HTTP头部
    evhttp_add_header(req->output_headers, "Content-Type", "application/json; charset=UTF-8");
	evhttp_add_header(req->output_headers, "Server", "my_httpd");
	//evhttp_add_header(req->output_headers, "Connection", "keep-alive");
	evhttp_add_header(req->output_headers, "Connection", "close");

    evbuffer *replyBuf = evbuffer_new();
	// 将要输出的值写入输出缓存
	if (msg.respone_data.length() > 0)
	{
        evbuffer_add(replyBuf, msg.respone_data.c_str(), msg.respone_data.length());
	}

    evhttp_send_reply(req, HTTP_OK, "OK", replyBuf);

    evbuffer_free(replyBuf);
}

int HttpServer::registerService()
{
    return 0;
}
