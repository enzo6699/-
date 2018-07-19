cmake_minimum_required(VERSION 2.8)

SET(PROJECT_NAME FaceSearchService)
PROJECT(${PROJECT_NAME})
SET (Demo_VERSION_APPNAME \"${PROJECT_NAME}\")
SET (Demo_VERSION_MAJOR 1)
SET (Demo_VERSION_MINOR 0)
SET (Demo_VERSION_MICRO 0)
SET (Demo_VERSION_REPOS 0)
set (CMAKE_SKIP_BUILD_RPATH TRUE)

# 加入一个配置头文件，用于处理 CMake 对源码的设置
configure_file (
    "config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/config.h"
    )

SET(THIRDPART_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../00-thirdparty/centos7)
SET(SER_COMMON_SROUCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../01-source)

INCLUDE_DIRECTORIES(
    ${THIRDPART_PATH}/uuid/include
    ${THIRDPART_PATH}/curl/include
    ${THIRDPART_PATH}/event/include
    ${THIRDPART_PATH}/faceall/include
    ${THIRDPART_PATH}/jsoncpp/include
    ${THIRDPART_PATH}/log4cpp/include
    ${THIRDPART_PATH}/opencv/include
    ${THIRDPART_PATH}/sqlite/include
    ${THIRDPART_PATH}/rdkafka/include
    ${SER_COMMON_SROUCE_PATH}
    ${SER_COMMON_SROUCE_PATH}/common/
    ${SER_COMMON_SROUCE_PATH}/utils/
    )

SET(SRC_LIST 
    ${SER_COMMON_SROUCE_PATH}/common/Lock.cpp
    ${SER_COMMON_SROUCE_PATH}/common/Logger.cpp
    ${SER_COMMON_SROUCE_PATH}/common/Properites.cpp
    ${SER_COMMON_SROUCE_PATH}/common/ConsumerImpl.cpp


    ${SER_COMMON_SROUCE_PATH}/utils/Base64Util.cpp
    ${SER_COMMON_SROUCE_PATH}/utils/FileUtil.cpp
    ${SER_COMMON_SROUCE_PATH}/utils/GuidUtil.cpp
    ${SER_COMMON_SROUCE_PATH}/utils/StringUtil.cpp
    ${SER_COMMON_SROUCE_PATH}/utils/TimeUtil.cpp
    ${SER_COMMON_SROUCE_PATH}/utils/HttpUtil.cpp
    ${SER_COMMON_SROUCE_PATH}/utils/ImageUtil.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../src/FaceSearchHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/BaseMsgHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/HttpServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/CodeStatus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/FaceAllIdentify.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/FaceDataConsumerCb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/FaceIdentify.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/SqliteFaceCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/PropertyConfigurator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/FaceAllIdentify.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/TimerThread.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp)

ADD_DEFINITIONS(-g)
LINK_DIRECTORIES(
    ${THIRDPART_PATH}/faceall/lib/
    ${THIRDPART_PATH}/curl/lib/
    ${THIRDPART_PATH}/event/lib/
    ${THIRDPART_PATH}/jsoncpp/lib/
    ${THIRDPART_PATH}/log4cpp/lib/
    ${THIRDPART_PATH}/opencv/lib/
    ${THIRDPART_PATH}/uuid/lib/
    ${THIRDPART_PATH}/sqlite/lib/
    ${THIRDPART_PATH}/rdkafka/lib/)

ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_LIST})

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    dl
    z
    uuid
    curl
    jsoncpp
    log4cpp
    opencv_core
    opencv_highgui
    opencv_imgproc
    rdkafka++
    rdkafka
    sqlite3
    faceall
    event pthread rt)

#std=c++
include(CheckCXXCompilerFlag)  
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)  
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)  
if(COMPILER_SUPPORTS_CXX11)  
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)  
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()  
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()  
#std=c++

