#include "FaceIdentify.h"
#include "PropertyConfigurator.h"
#include <log4cpp/Category.hh>
#include "CodeStatus.h"
#include "utils/HttpUtil.h"
#include "utils/Base64Util.h"
#include "utils/ImageUtil.h"
#include "utils/TimeUtil.h"
#include "utils/GuidUtil.h"

#define MAX_IMAGE_SIZE 1024 * 1024 * 4

static char imageBuffer[MAX_IMAGE_SIZE];
// 单次最大获取的记录数量
static const int maxRowsOnce = 10000;
static PropertyConfigurator &config = PropertyConfigurator::Instance();
static log4cpp::Category &logger = log4cpp::Category::getRoot();

FaceIdentify& FaceIdentify::Instance()
{
	static FaceIdentify faceIdentify;
	return faceIdentify;
}

FaceIdentify::FaceIdentify()
{
}

FaceIdentify::~FaceIdentify()
{
	if (m_pFaceAllIdentify != NULL)
	{
		delete m_pFaceAllIdentify;
		m_pFaceAllIdentify = NULL;
	}
}

int FaceIdentify::Init(SqliteFaceCache *pSqliteFaceCache)
{
	int nRet = 0;
    m_pFaceAllIdentify = new tce::FaceAllIdentify();
	m_pSqliteFaceCache = pSqliteFaceCache;

	nRet = m_pFaceAllIdentify->Init(tce::face_type_e::face_all);
	if (nRet != 0)
	{
		log4cpp::Category::getRoot().error("sdk init failed,nRet:%d", nRet);
		return 1;
	}

	return 0;
}

void FaceIdentify::TestSqlite()
{
	int nRet = 0;
	char destbuf[10] = { 0 };
	for (size_t i = 0; i < 8; i++)
	{
		sprintf(destbuf, "%d", i);
		std::string path = "./image/";
		path.append(destbuf);
		path.append(".JPG");
        int length = MAX_IMAGE_SIZE;
		if (tce::ImageUtil::ReadFile(path, imageBuffer, length) == 0)
		{
            std::vector<std::shared_ptr<tce::FaceResult>> vecFaceDetailed;
			m_pFaceAllIdentify->Feature(imageBuffer, length, vecFaceDetailed);
			if (vecFaceDetailed.size() > 0)
			{
				// 插入数据库
				std::vector<std::shared_ptr<face_info_t>> vecFaceInfo;
				for (int i = 0; i < 1000; i++)
				{
					std::shared_ptr<face_info_t> face_info(new face_info_t());
					face_info->cameraId = "1";
					face_info->featureLength = vecFaceDetailed.at(0)->m_feature.length * sizeof(float);
					face_info->feature = new float[vecFaceDetailed.at(0)->m_feature.length];
					memcpy(face_info->feature, vecFaceDetailed.at(0)->m_feature.feature, face_info->featureLength);

					face_info->objectUrl = path;
					face_info->originalUrl = path;
					face_info->timestamp = tce::TimeUtil::GetTimestamp_s();
					face_info->personId = tce::GuidUtil::gguid();

					vecFaceInfo.push_back(face_info);
				}

				int64_t start = tce::TimeUtil::GetTimestamp();
//				if ((nRet = m_pSqliteFaceCache->Insert(vecFaceInfo)) != 0)
//				{
//					log4cpp::Category::getRoot().error("sqlite insert error ,nRet:%d", nRet);
//				}
				int64_t end = tce::TimeUtil::GetTimestamp();
				log4cpp::Category::getRoot().info("sqlite batch insert %d speed:%d", vecFaceInfo.size(), (end - start));
				vecFaceInfo.clear();
			}
		}
		else
		{
			log4cpp::Category::getRoot().error("image not find %s", path.c_str());
		}
	}
}

void FaceIdentify::TestSqliteQuery()
{
	int row_num = 0;
	int count = 0;
	int rc = 0;
	sqlite3_stmt *stmt = 0;
	int tempCount = 0;
	float confidence = 0.0f;

	face_search_req_t face_search_req;
	face_search_req.startTime = 1522749165;
	face_search_req.endTime = 1622749279;
	face_search_req.vecCameraId.push_back("1");

	int64_t start = tce::TimeUtil::GetTimestamp();
	// sqlite查询结果和比对
	std::string sqlcmd = m_pSqliteFaceCache->Search(face_search_req.startTime, face_search_req.endTime, face_search_req.vecCameraId);
	rc = sqlite3_prepare_v2(m_pSqliteFaceCache->m_db, sqlcmd.c_str(), -1, &stmt, NULL);
	if (SQLITE_OK != rc) {
		log4cpp::Category::getRoot().error("cat't prepare:%s", sqlite3_errstr(rc));
		return;
	}

	do{
		sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, maxRowsOnce);
		sqlite3_bind_int(stmt, 2, row_num);
		rc = sqlite3_step(stmt);
		while (rc == SQLITE_ROW)
		{
			compare_result_t *pCompare_result = new compare_result_t();
			pCompare_result->confidence = confidence;
			pCompare_result->personId = (const char*)(sqlite3_column_text(stmt, 0));
			pCompare_result->cameraId = (const char*)sqlite3_column_text(stmt, 1);
			pCompare_result->originalUrl = (const char*)sqlite3_column_text(stmt, 2);
			pCompare_result->objectUrl = (const char*)sqlite3_column_text(stmt, 3);
			const void* featureB = sqlite3_column_blob(stmt, 4);
			pCompare_result->timestamp = sqlite3_column_int(stmt, 5);
			rc = rc = sqlite3_step(stmt);
			tempCount++;
		}

        if (tempCount < maxRowsOnce)
			break;// 已经获取完数据

		tempCount = 0;
		count++;
        row_num = count*maxRowsOnce;
	} while (true);

	rc = sqlite3_finalize(stmt);
	int64_t end = tce::TimeUtil::GetTimestamp();
	log4cpp::Category::getRoot().info("sqlite query %d speed:%d", row_num + tempCount, (end - start));

	return;
}

void FaceIdentify::TestCompare()
{
	int nRet = 0;
	face_search_req_t face_search_req;
	face_search_req.startTime = 1522749165;
	face_search_req.endTime = 1622749279;
	std::string path = "./image/1.JPG";
    int length = MAX_IMAGE_SIZE;
	tce::ImageUtil::ReadFile(path, imageBuffer, length);
	face_search_req.image = tce::Base64Util::base64_encode(imageBuffer, length);
	face_search_req.minConfidence = 0.5f;
	face_search_req.topN = 11;
	face_search_req.vecCameraId.push_back("1");
    std::vector<std::shared_ptr<compare_result_t>> resultList;
	int64_t start = tce::TimeUtil::GetTimestamp();
	nRet = Search(face_search_req, resultList);
	int64_t end = tce::TimeUtil::GetTimestamp();
	log4cpp::Category::getRoot().info("speed:%d", end - start);
	if (nRet != 0)
	{
		log4cpp::Category::getRoot().error("Search error ,nRet:%d", nRet);
	}
	else
	{
		std::list<compare_result_t*>::iterator it;
        for (auto pTem: resultList)
		{
			if (pTem != NULL)
				log4cpp::Category::getRoot().info("result:%s, %f", pTem->personId.c_str(), pTem->confidence);
		}
	}
}

/*
* 降序排序
*/
void FaceIdentify::sortAndInsert(std::vector<std::shared_ptr<compare_result_t>> &list, std::shared_ptr<compare_result_t> pCompare_result, size_t maxSize)
{
    size_t index = 0;
    for (index = 0; index < list.size(); index++)
	{
        auto &result = list[index];
        if (pCompare_result->confidence > result->confidence)
		{
			// 相似度大于当前值，在当前值前面插入元素
            list.insert(list.begin() + index, pCompare_result);
            break;
		}
	}

    if (index == list.size()) {
        list.push_back(pCompare_result);
    }

	// 如果插入未成功，检测当前数量，如果不足则插入末尾
    if (list.size() > maxSize)
	{
        list.pop_back();
	}
}

int FaceIdentify::Search(const face_search_req_t& face_search_req, std::vector<std::shared_ptr<compare_result_t>> &resultList)
{
	// 抓拍库检索
    int rowNum = 0;
	int count = 0;
    int tempCount = 0;
    int rc = 0;
	sqlite3_stmt *stmt = 0;
	float confidence = 0.0f;

	// 获取人脸特征值
	const char *imageData = NULL;
	std::string imageDataStr;
    int imageLength = MAX_IMAGE_SIZE;
	if (face_search_req.image.length() > 0)
	{
		imageDataStr = tce::Base64Util::base64_decode(face_search_req.image);
		imageLength = imageDataStr.length();
		imageData = imageDataStr.c_str();
	}
	else
	{
		if (face_search_req.url.length() == 0)
		{
			log4cpp::Category::getRoot().error(createError(ErrorCode::image_data_is_empty, "image is null and url is null"));
			return ErrorCode::image_data_is_empty;
		}

		imageLength = tce::HttpUtil::HttpGet(face_search_req.url.c_str(), imageBuffer, imageLength);
		imageData = imageBuffer;
	}

	if (imageLength == 0)
	{
		//图片数据为空
		log4cpp::Category::getRoot().error(createError(ErrorCode::image_data_is_empty, "image data is null"));
		return ErrorCode::image_data_is_empty;
	}

	//  人脸特征提取
    std::vector<std::shared_ptr<tce::FaceResult>> vecFaceDetailed;
	rc = m_pFaceAllIdentify->Feature(imageData, imageLength, vecFaceDetailed);
	if (rc != 0)
	{
		log4cpp::Category::getRoot().error(createError(ErrorCode::image_format_error, ""));
		return ErrorCode::image_format_error;
	}

	if (vecFaceDetailed.size() == 0)
	{
		log4cpp::Category::getRoot().error(createError(ErrorCode::no_faces_in_image, ""));
		return ErrorCode::no_faces_in_image;
	}

	float* featureA = vecFaceDetailed.at(0)->m_feature.feature;
	int featureALength = vecFaceDetailed.at(0)->m_feature.length;

	// sqlite查询结果和比对
	std::string sqlcmd = m_pSqliteFaceCache->Search(face_search_req.startTime, face_search_req.endTime, face_search_req.vecCameraId);
	rc = sqlite3_prepare_v2(m_pSqliteFaceCache->m_db, sqlcmd.c_str(), -1, &stmt, NULL);
	if (SQLITE_OK != rc) {
		log4cpp::Category::getRoot().error("cat't prepare:%s", sqlite3_errstr(rc));
		return ErrorCode::read_loacl_faceinfo_error;
	}

	do{
		sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, maxRowsOnce);
        sqlite3_bind_int(stmt, 2, rowNum);

        tempCount = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW)
		{
            tempCount++;

            const char *personId = (const char*)(sqlite3_column_text(stmt, 0));
            const char *cameraId = (const char*)sqlite3_column_text(stmt, 1);
            const char *originalUrl = (const char*)sqlite3_column_text(stmt, 2);
            const char *objectUrl = (const char*)sqlite3_column_text(stmt, 3);
            const void *featureB = sqlite3_column_blob(stmt, 4);
            double quality = sqlite3_column_double(stmt, 5);
            int timestamp = sqlite3_column_int(stmt, 6);

            if (quality < config.qualityThreshold) {
                logger.debug("face quality too low, collectFaceId: %s", personId);
                continue;
            }

            confidence = 0.0f;
			m_pFaceAllIdentify->Compare(featureA, featureALength, (float *)featureB, featureALength, confidence);
            if ((float)confidence >= face_search_req.minConfidence) {
                std::shared_ptr<compare_result_t> compare_result(new compare_result_t);
                compare_result->confidence = confidence;
                compare_result->personId = personId;
                compare_result->cameraId = cameraId;
                compare_result->originalUrl = originalUrl;
                compare_result->objectUrl = objectUrl;
                compare_result->timestamp = timestamp;

                sortAndInsert(resultList, compare_result, face_search_req.topN);
            } else {
                logger.debug("face confidence too low, collectFaceId: %s", personId);
            }
		}

        if (tempCount < maxRowsOnce) {
            logger.info("Finished query, count: %d, last rows: %d", count, tempCount);
            break;// 已经获取完数据
        }

		count++;
        rowNum = count * maxRowsOnce;
	} while (true);

    log4cpp::Category::getRoot().info("search total:%d", resultList.size());

    rc = sqlite3_finalize(stmt);

	return 0;
}
