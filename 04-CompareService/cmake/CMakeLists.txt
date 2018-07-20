cmake_minimum_required(VERSION 2.8)

SET(PROJECT_NAME CompareService)
PROJECT(${PROJECT_NAME})
SET (Demo_VERSION_APPNAME \"${PROJECT_NAME}\")
SET (Demo_VERSION_MAJOR 1)
SET (Demo_VERSION_MINOR 0)
SET (Demo_VERSION_MICRO 0)
SET (Demo_VERSION_REPOS 0)
set (CMAKE_SKIP_BUILD_RPATH TRUE)

option(USE_SEETA "使用中科视拓算法，否则使用飞搜算法" OFF)

ADD_DEFINITIONS(-std=c++11)

SET(THIRDPART_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../00-thirdparty/centos7)
SET(COMMON_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../01-source)

set(INCLUDE_DIRS
    ${THIRDPART_DIR}/event/include
    ${THIRDPART_DIR}/curl/include
    ${THIRDPART_DIR}/jsoncpp/include
    ${THIRDPART_DIR}/opencv/include
    ${THIRDPART_DIR}/rdkafka/include
    ${THIRDPART_DIR}/log4cpp/include
    ${COMMON_SOURCE_PATH}/common/
    ${COMMON_SOURCE_PATH}/utils/
    )

if(USE_SEETA)
    INCLUDE_DIRECTORIES(${THIRDPART_DIR}/seeta/include ${INCLUDE_DIRS})
    add_definitions("-DUSE_SEETA")
    message(STATUS "使用中科视拓算法")
else()
    INCLUDE_DIRECTORIES(${THIRDPART_DIR}/faceall/include ${INCLUDE_DIRS})
    message(STATUS "使用飞搜算法")
endif()

SET(LIBS
    faceall
    opencv_core
    opencv_highgui
    opencv_imgproc
    uuid
    jsoncpp
    event
    event_core
    event_extra
    event_pthreads
    rdkafka++
    rdkafka
    pthread
    log4cpp
    curl
    rt
    dl
    z)

SET(SRC_LIST 
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/FaceEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/CompareConsumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/DataCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/HttpService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/PropertyConfigurator.cpp

    ${COMMON_SOURCE_PATH}/common/Properites.cpp
    ${COMMON_SOURCE_PATH}/common/ProducerImpl.cpp
    ${COMMON_SOURCE_PATH}/common/ConsumerImpl.cpp
    ${COMMON_SOURCE_PATH}/utils/StringUtil.cpp
    ${COMMON_SOURCE_PATH}/utils/GuidUtil.cpp
    ${COMMON_SOURCE_PATH}/utils/Base64Util.cpp
    )

ADD_DEFINITIONS(-Wall)
LINK_DIRECTORIES(
    ${THIRDPART_DIR}/faceall/lib
    ${THIRDPART_DIR}/event/lib
    ${THIRDPART_DIR}/opencv/lib
    ${THIRDPART_DIR}/log4cpp/lib
    ${THIRDPART_DIR}/curl/lib
    ${THIRDPART_DIR}/jsoncpp/lib
    ${THIRDPART_DIR}/rdkafka/lib)

ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_LIST})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBS})
