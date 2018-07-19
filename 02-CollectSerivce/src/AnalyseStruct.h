#ifndef _ANALYSE_STRUCT_H_
#define _ANALYSE_STRUCT_H_

#include <memory>
#include "Media.h"
#include "Faceall.h"

struct FaceInfo
{
    FaceInfo();
    ~FaceInfo();

    std::shared_ptr<tce::FaceResult> detail;
    std::string originalPath;
    std::string objectPath;
    int64_t timestamp = 0;
    float age;
    int race;
    int gender;
};

#endif
