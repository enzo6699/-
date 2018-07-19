#ifndef _Face_Identify_H
#define _Face_Identify_H

#include <vector>
#include <list>
#include "SqliteFaceCache.h"
#include "FaceAllIdentify.h"

typedef struct _face_search_req_t {
	int startTime;
	int endTime;
	int topN;
	float minConfidence;
	std::string personType;
	std::string url;
	std::string image;
	std::vector<std::string> vecCameraId;
} face_search_req_t;

typedef struct _compare_result_t {
	std::string personId;
	std::string cameraId;
	std::string originalUrl;
	std::string objectUrl;
	int timestamp;
	float confidence;
}compare_result_t;

class FaceIdentify
{
public:
	static FaceIdentify& Instance();
public:
	FaceIdentify();
	~FaceIdentify();

public:
    int Init(SqliteFaceCache *sqliteFaceCache);

	/*
	* 抓拍库检索
	*/
    int Search(const face_search_req_t &face_search_req, std::vector<std::shared_ptr<compare_result_t>> &resultList);

	void TestSqlite();

	void TestSqliteQuery();

	void TestCompare();

private:
    void sortAndInsert(std::vector<std::shared_ptr<compare_result_t>> &list, std::shared_ptr<compare_result_t> pCompare_result, size_t maxSize);

private:
	SqliteFaceCache *m_pSqliteFaceCache;
	tce::FaceAllIdentify* m_pFaceAllIdentify;
};

#endif
