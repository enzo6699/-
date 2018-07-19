#include "SqliteFaceCache.h"
#include "PropertyConfigurator.h"
#include <memory>
#include <stdio.h>
#include <iostream>
#include <log4cpp/Category.hh>
#include "utils/TimeUtil.h"

// 创建数据库
static const char* sql_create_data = "CREATE TABLE IF NOT EXISTS FACEINFO( "
        "personId VARCHAR(32) PRIMARY KEY, "
        "cameraId VARCHAR(32) not null, "
        "originalUrl VARCHAR(255) not null, "
        "objectUrl VARCHAR(255) not null, "
        "feature BLOB not null, "
        "objectRect VARCHAR(32) not null, "
        "objectQuality REAL not null, "
        "timestamp INTEGER not null"
        ");";

static const char* sql_insert_data = "INSERT INTO FACEINFO VALUES(?,?,?,?,?,?,?,?);";
static const char* sql_delete_data = "DELETE FROM FACEINFO WHERE personId=?;";

//将内存数据库中的信息同步到文件数据库中
static std::string sql_transfer_data;
static log4cpp::Category &logger = log4cpp::Category::getRoot();

SqliteFaceCache::SqliteFaceCache() : mRunning(false)
{
}

SqliteFaceCache::~SqliteFaceCache()
{
    mRunning = false;
    deleter->join();

    if (m_db != NULL)
	{
        logger.info("close db");
        sqlite3_close_v2(m_db);
        m_db = NULL;
	}
}

int SqliteFaceCache::init(const char *filename)
{
    if (!filename) {
        logger.error("filename is null");
        return -1;
    }

	char *zErrMsg = 0;
	int rc;

    logger.info("create sqlite db: filename:%s", filename);

    rc = sqlite3_open_v2(filename, &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);

	if (SQLITE_OK != rc) {
        log4cpp::Category::getRoot().error("cat't open database:%s", sqlite3_errmsg(m_db));
        sqlite3_close_v2(m_db);
		return -1;
	}

    rc = sqlite3_exec(m_db, sql_create_data, NULL, NULL, &zErrMsg);
	if (SQLITE_OK != rc) {
        log4cpp::Category::getRoot().error("cat't create table:%s", zErrMsg);
		sqlite3_free(zErrMsg);
        sqlite3_close_v2(m_db);
		return -1;
	}

    rc = sqlite3_exec(m_db, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);
    if (SQLITE_OK != rc) {
        logger.error("cat't set async mode: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close_v2(m_db);
        return -1;
    }

    rc = sqlite3_exec(m_db, "PRAGMA journal_mode = MEMORY", NULL, NULL, &zErrMsg);
    if (SQLITE_OK != rc) {
        logger.error("cat't set memory journal: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close_v2(m_db);
        return -1;
    }

    logger.info("succeeded init database, sqlite version: %s", sqlite3_version);
    logger.info("start deleter thread");
    deleter = std::make_shared<std::thread>(&SqliteFaceCache::checkOverdue, this);

	return 0;
}

int SqliteFaceCache::Insert(const char* personId,
                            const char* cameraId,
                            const char* originalUrl,
                            const char* objectUrl,
                            const std::string &feature,
                            const std::string &objectRect,
                            float objectQuality,
                            int timestamp)
{
	int rc = 0;
	sqlite3_stmt *stmt = 0;

    rc = sqlite3_prepare_v2(m_db, sql_insert_data, -1, &stmt, NULL);
	if (SQLITE_OK != rc) {
		log4cpp::Category::getRoot().error("cat't prepare:%s", sqlite3_errstr(rc));
		return -1;
	}

	rc = sqlite3_bind_text(stmt, 1, personId, -1, SQLITE_STATIC); //绑定数据
	rc = sqlite3_bind_text(stmt, 2, cameraId, -1, SQLITE_STATIC); //绑定数据
	rc = sqlite3_bind_text(stmt, 3, originalUrl, -1, SQLITE_STATIC); //绑定数据
	rc = sqlite3_bind_text(stmt, 4, objectUrl, -1, SQLITE_STATIC); //绑定数据
    rc = sqlite3_bind_blob(stmt, 5, feature.c_str(), feature.length(), SQLITE_STATIC); //绑定数据
    rc = sqlite3_bind_text(stmt, 6, objectRect.c_str(), -1, SQLITE_STATIC); //绑定数据
    rc = sqlite3_bind_double(stmt, 7, objectQuality); //绑定数据
    rc = sqlite3_bind_int(stmt, 8, timestamp); //绑定数据
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return -2;
	rc = sqlite3_finalize(stmt);

	return 0;
}

/*
int SqliteFaceCache::Insert(std::vector<std::shared_ptr<face_info_t>> &vecFaceInfo)
{
	int rc = 0;
	sqlite3_stmt *stmt = 0;

	size_t vecSize = vecFaceInfo.size();
	if (vecSize == 0)
		return 0;

    sqlite3_exec(m_db, "BEGIN;", 0, 0, 0);
    rc = sqlite3_prepare_v2(m_db, sql_insert_data, -1, &stmt, NULL);
	if (SQLITE_OK != rc) {
        rc = sqlite3_exec(m_db, "COMMIT;", 0, 0, 0);
		log4cpp::Category::getRoot().error("cat't prepare:%s", sqlite3_errstr(rc));
		return -1;
	}

	for (size_t i = 0; i < vecSize; i++)
	{
		sqlite3_reset(stmt);
		rc = sqlite3_bind_text(stmt, 1, vecFaceInfo.at(i)->personId.c_str(), -1, SQLITE_STATIC); //绑定数据
		rc = sqlite3_bind_text(stmt, 2, vecFaceInfo.at(i)->cameraId.c_str(), -1, SQLITE_STATIC); //绑定数据
		rc = sqlite3_bind_text(stmt, 3, vecFaceInfo.at(i)->originalUrl.c_str(), -1, SQLITE_STATIC); //绑定数据
		rc = sqlite3_bind_text(stmt, 4, vecFaceInfo.at(i)->objectUrl.c_str(), -1, SQLITE_STATIC); //绑定数据
		rc = sqlite3_bind_blob(stmt, 5, vecFaceInfo.at(i)->feature, vecFaceInfo.at(i)->featureLength, SQLITE_STATIC); //绑定数据
		rc = sqlite3_bind_int(stmt, 6, vecFaceInfo.at(i)->timestamp); //绑定数据
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
            rc = sqlite3_exec(m_db, "COMMIT;", 0, 0, 0);
			return -2;
		}
	}
	rc = sqlite3_finalize(stmt);
    rc = sqlite3_exec(m_db, "COMMIT;", 0, 0, 0);

	if (SQLITE_OK != rc) {
		log4cpp::Category::getRoot().error("cat't prepare:%s", sqlite3_errstr(rc));
		return -3;
	}

	return 0;
}
*/

std::string SqliteFaceCache::Search(int startTime, int endTime, std::vector<std::string> vecCameraId)
{
	std::stringstream sqlcmd;
    sqlcmd << "SELECT personId, cameraId, originalUrl, objectUrl, feature, objectQuality, timestamp FROM FACEINFO WHERE 1=1 ";

	if (startTime != 0)
		sqlcmd << " AND timestamp >= " << startTime;

	if (endTime != 0)
		sqlcmd << " AND timestamp <= " << endTime;

	size_t size = vecCameraId.size();
	bool bFirst = true;
	for (size_t i = 0; i < size; i++)
	{
		if (bFirst)
		{
			sqlcmd << " AND cameraId IN (";
			bFirst = false;
			sqlcmd << "'" << vecCameraId.at(i) << "'";
		}
		else
		{
			sqlcmd << "," << "'" << vecCameraId.at(i) << "'";
		}
	}
	if (size > 0)
	{
		sqlcmd << ")";
	}

	sqlcmd << " LIMIT ? OFFSET ? ";

	std::string sqlcmdStr = sqlcmd.str();
	log4cpp::Category::getRoot().info("search sqlcmd:%s", sqlcmdStr.c_str());

	return sqlcmdStr;
}

int SqliteFaceCache::SearchCount(int startTime,
	int endTime,
	std::vector<std::string> vecCameraId)
{
	int rc = 0;
	sqlite3_stmt *stmt = 0;
	char **dbResult;
	int total;     //nRow 查找出的总行数,nColumn 存储列 

	std::stringstream sqlcmd;
	sqlcmd << "SELECT COUNT(*) FROM FACEINFO WHERE 1=1 ";

	if (startTime != 0)
		sqlcmd << " AND timestamp >= " << startTime;

	if (endTime != 0)
		sqlcmd << " AND timestamp <= " << endTime;

	size_t size = vecCameraId.size();
	bool bFirst = true;
	for (size_t i = 0; i < size; i++)
	{
		if (bFirst)
		{
			sqlcmd << " AND cameraId IN (";
			bFirst = false;
			sqlcmd << "'" << vecCameraId.at(i) << "'";
		}
		else
		{
			sqlcmd << "," << "'" << vecCameraId.at(i) << "'";
		}
	}
	if (size > 0)
	{
		sqlcmd << ")";
	}

	std::string sqlcmdStr = sqlcmd.str();
	log4cpp::Category::getRoot().info("search sqlcmd:%s", sqlcmdStr.c_str());


    rc = sqlite3_prepare_v2(m_db, sqlcmdStr.c_str(), sqlcmdStr.length(), &stmt, NULL);
	if (SQLITE_OK != rc) {
		log4cpp::Category::getRoot().error("cat't prepare:%s", sqlite3_errstr(rc));
		return -1;
	}

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW)
		return -2;
	total = sqlite3_column_int(stmt, 0);
	rc = sqlite3_finalize(stmt);
	return total;
}

int SqliteFaceCache::Delete(const char* personId)
{
	int rc = 0;
	sqlite3_stmt *stmt = 0;

    rc = sqlite3_prepare_v2(m_db, sql_delete_data, -1, &stmt, NULL);
	if (SQLITE_OK != rc) {
		log4cpp::Category::getRoot().error("cat't prepare:%s", sqlite3_errstr(rc));
		return -1;
	}

	rc = sqlite3_bind_text(stmt, 1, personId, -1, SQLITE_STATIC); //绑定数据
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return -2;
	rc = sqlite3_finalize(stmt);

	return 0;
}

int SqliteFaceCache::DeleteOverdue(int retentionTime)
{
	int rc = 0;
	sqlite3_stmt *stmt = 0;

    rc = sqlite3_prepare_v2(m_db, "DELETE FROM FACEINFO WHERE timestamp<?;", -1, &stmt, NULL);
	if (SQLITE_OK != rc) {
		log4cpp::Category::getRoot().error("cat't prepare:%s", sqlite3_errstr(rc));
		return -1;
	}

	int timestamp = tce::TimeUtil::GetTimestamp_s() - retentionTime;

	rc = sqlite3_bind_int(stmt, 1, timestamp); //绑定数据
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return -2;
	rc = sqlite3_finalize(stmt);
    return 0;
}

void SqliteFaceCache::checkOverdue()
{
    mRunning = true;
    while (mRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        time_t now = time(0);
        tm *ltm = localtime(&now);
        // 每天凌晨2点进行数据清理
        if (ltm->tm_hour == 2)
        {
            logger.info("Delete Overdue Data start");
            // 保留一个月数据
            int rc = DeleteOverdue(PropertyConfigurator::Instance().sqliteCachetime * 24 * 3600);
            if (rc != 0)
            {
                logger.error("Delete Overdue Data error {%d}", rc);
            }
            logger.info("Delete Overdue Data end");
        }
    }
}
