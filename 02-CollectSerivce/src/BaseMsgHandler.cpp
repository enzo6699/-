#include "BaseMsgHandler.h"

BaseMsgHandler::BaseMsgHandler() :
    logger(log4cpp::Category::getRoot()),
    taskMgr(TaskMgr::Instance()),
    config(PropertyConfigurator::Instance())
{
}

BaseMsgHandler::~BaseMsgHandler()
{
}
