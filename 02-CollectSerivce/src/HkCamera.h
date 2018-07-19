#ifndef _HK_CAMERA_H_
#define _HK_CAMERA_H_

#include "Media.h"
#include <string>
#include <HCNetSDK.h>
#ifdef _MSC_VER
#include <plaympeg4.h>
#else
#include <PlayM4.h>
#endif
#include <log4cpp/Category.hh>

class CameraTask;

class HKCamera : public Media
{
public:
    HKCamera(CameraTask *task);
	~HKCamera();

    int Start(const std::string &ip, int port, const std::string &username, const std::string &password);
	void Stop();

private:
    static void CALLBACK realDataCallback(LONG playHandle, DWORD dataType, BYTE *buffer, DWORD bufferSize, void *userData);
    static void CALLBACK displayCallback(DISPLAY_INFO *displayInfo);
    static void alarmMessageCallback(LONG command, NET_DVR_ALARMER *alarmer, char *alarmInfo, DWORD bufLen, void *user);
    int login();
	void capture(const DISPLAY_INFO *displayInfo);
    bool initFaceConfig();

    long m_userID = -1;
    long m_playHandle = -1;
    int m_playPort = -1;
    NET_DVR_DEVICEINFO_V30 deviceInfo;

	std::string m_ip;
	unsigned short m_port;
	std::string m_username;
	std::string m_password;

    CameraTask *task = NULL;
    log4cpp::Category &logger;
};


#endif
