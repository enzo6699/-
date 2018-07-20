#include "DataCache.h"
#include "Base64Util.h"

using namespace std;

static bool cmp(const shared_ptr<SearchResult> &r1, const shared_ptr<SearchResult> &r2)
{
    return r1->confidence > r2->confidence;
}

DataCache::DataCache() :
    logger(log4cpp::Category::getRoot()),
    config(PropertyConfigurator::Instance())
{
    faceall = std::make_shared<tce::FaceEngine>();
    faceall->init(tce::FaceCompare);
}

DataCache::~DataCache()
{
}

int DataCache::remove(const string &personId, const std::vector<string> &faceIds)
{
    lock.lock();
    if (persons.find(personId) == persons.end()) {
        logger.error("No such person: %s", personId.c_str());
        errorMessage = "personId(" + personId + ")不存在";
        lock.unlock();
        return 0;
    }

    int count = 0;
    if (faceIds.size() == 0) {
        count = persons[personId]->faces.size();
        logger.info("Removing all faces of person: %s", personId.c_str());
        persons.erase(personId);
    } else {
        shared_ptr<Person> person = persons[personId];
        for (size_t index = 0; index < faceIds.size(); index++) {
            string faceId = faceIds[index];

            if (person->faces.find(faceId) == person->faces.end()) {
                logger.error("No such face: %s in person: %s", faceId.c_str(), personId.c_str());
                errorMessage = "faceId(" + faceId + ")不存在";
                lock.unlock();
                return 0;
            }
            count++;

            logger.info("Removing face: %s in person: %s", faceId.c_str(), personId.c_str());
            person->faces.erase(faceId);
        }
    }
    lock.unlock();

    return count;
}

int DataCache::updatePerson(const string &personId, const string &personType, const string &personName)
{
    int result = -1;
    lock.lock();
    if (persons.find(personId) != persons.end()) {
        std::shared_ptr<Person> target = persons[personId];
        target->personType = personType;
        target->personName = personName;
        result = 0;
    }
    lock.unlock();

    return result;
}

int DataCache::add(std::shared_ptr<DataCache::Person> person)
{
    lock.lock();
    if (person.get() == NULL) {
        lock.unlock();
        logger.error("Can't add empty person");
        return 0;
    }

    int count = 0;
    string personId = person->personId;
    if (persons.find(personId) != persons.end()) {
        shared_ptr<Person> oldPerson = persons[personId];
        for (auto iter: person->faces) {
            string newFaceId = iter.first;
            if (oldPerson->faces.find(newFaceId) != oldPerson->faces.end()) {
                logger.error("Face: %s of person: %s already exist", newFaceId.c_str(), personId.c_str());
                count = 0;
                break;
            } else {
                oldPerson->faces[newFaceId] = person->faces[newFaceId];
                count++;
            }
        }
    } else {
        persons[personId] = person;
        count = person->faces.size();
    }

    lock.unlock();
    return count;
}

std::vector<std::shared_ptr<SearchResult>> DataCache::findFace(const std::vector<float> &feature, const string &personType, int topN)
{
    std::vector<std::shared_ptr<SearchResult>> candidates;

    lock.lock();

    for (auto it: persons) {
        string personId = it.first;
        shared_ptr<Person> person = it.second;
        if (person->personType != personType && personType.size() > 0) {
            continue;
        }

        for (auto it2: person->faces) {
            string faceId = it2.first;
            shared_ptr<Face> face = it2.second;

            string feature1 = tce::Base64Util::base64_decode(face->feature);
            string feature1Start = face->feature.substr(0, 16);
            string feature1End = face->feature.substr(face->feature.size() - 16, 16);
            float similarity = 0.0f;
            float *start = (float*)feature1.c_str();
            vector<float> feature11(start, start + feature1.size() / sizeof(float));
            faceall->compare(feature11, feature, similarity);

            logger.debug("Compare, id: %s, type: %s, faceId: %s, feature: %s...%s, confidence: %.2f",
                         person->personId.c_str(),
                         person->personType.c_str(),
                         faceId.c_str(),
                         feature1Start.c_str(),
                         feature1End.c_str(),
                         similarity);

            std::shared_ptr<SearchResult> result(new SearchResult);
            result->personId = personId;
            result->personType = person->personType;
            result->personName = person->personName;
            result->faceId = faceId;
            result->objectUrl = face->objectUrl;
            result->confidence = similarity;
            candidates.push_back(result);
        }
    }

    std::sort(candidates.begin(), candidates.end(), cmp);

    std::vector<std::shared_ptr<SearchResult>> results;
    for (size_t index = 0; index < topN; index++) {
        if (candidates.size() > index) {
            results.push_back(candidates[index]);
        }
    }

    lock.unlock();

    return results;
}

std::vector<std::string> DataCache::getTypes()
{
    lock.lock();
    std::vector<std::string> types;
    for (auto it: persons) {
        shared_ptr<Person> person = it.second;
        if (std::find(types.begin(), types.end(), person->personType) == types.end()) {
            types.push_back(person->personType);
        }
    }
    lock.unlock();
    return types;
}
