#include "AnalyseStruct.h"

FaceInfo::FaceInfo()
{
}

FaceInfo::~FaceInfo()
{
    remove(originalPath.c_str());
    remove(objectPath.c_str());
}
