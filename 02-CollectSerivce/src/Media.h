#ifndef _FACAMERA_H_
#define _FACAMERA_H_

#include <memory>
#include <string>
#include <opencv2/core/core.hpp>
#include <TimeUtil.h>

struct FrameInfo
{
    FrameInfo()
    {
        timestamp = tce::TimeUtil::GetTimestamp();
    }

    cv::Mat faceMat;
    cv::Mat mat;
    cv::Rect roi;
    int64_t timestamp = 0; // ms
};

class Media
{
public:
	Media();
	~Media();

public:
    virtual int Start(const std::string &ip, int port, const std::string &username, const std::string &password) { return 0; }
    virtual int Start(const std::string &url) { return 0; }
	virtual void Stop() = 0;
};


#endif
