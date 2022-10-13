#include <iostream>
#include <vector>
#include <algorithm>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <curlwrapper.hpp>

//std::string base_url = "http://localhost:8000/api/cdb_rest/";
std::string base_url = "http://linostest.apps.usatlas.bnl.gov/api/cdb_rest/";

namespace backend {

std::vector<std::string> _getItemNames(nlohmann::json j) {
    std::vector<std::string> name_list;
    for (const auto& obj: j){
        name_list.push_back(obj["name"]);
    }
    return name_list;
}

// Reading
nlohmann::json getGlobalTags() {
    return curlwrapper::get(base_url + "globalTags");
}

nlohmann::json getGlobalTagStatuses() {
    return curlwrapper::get(base_url + "gtstatus");
}

nlohmann::json getGlobalTagTypes() {
    return curlwrapper::get(base_url + "gttype");
}

nlohmann::json getPayloadTypes() {
    return curlwrapper::get(base_url + "pt");
}

nlohmann::json getPayloadLists() {
    return curlwrapper::get(base_url + "pl");
}

nlohmann::json getPayloadLists(std::string gtName) {
    return curlwrapper::get(base_url + "gtPayloadLists/" + gtName);
}

nlohmann::json getGlobalTagMap(std::string gtName){
    return curlwrapper::get(base_url + "globalTag/" + gtName);
}

nlohmann::json getPayloadIOVs(std::string gtName, int minorIov, int majorIov){
    return curlwrapper::get(base_url + "payloadiovs/?gtName=" + gtName + "&majorIOV=" + std::to_string(majorIov) + "&minorIOV=" + std::to_string(minorIov));
    //return curlwrapper::get(base_url + "payloadiovs/?gtName=" + gtName);
}

nlohmann::json getPayloadIOVs(std::string gtName, int minorIov){
    return curlwrapper::get(base_url + "payloadiovs/?gtName=" + gtName + "&majorIOV=0&minorIOV=" + std::to_string(minorIov));
}

bool gtExists(std::string gtName){
    std::vector<std::string> gtns = _getItemNames(getGlobalTags());
    return std::find(gtns.begin(), gtns.end(), gtName) != gtns.end();
}

bool plTypeExists(std::string plType){
    std::vector<std::string> ptns = _getItemNames(getPayloadTypes());
    return std::find(ptns.begin(), ptns.end(), plType) != ptns.end();
}

void checkGtExists(std::string gtName){
    if (!gtExists(gtName)){
        std::cout<<"No global tag with name '"<<gtName<<"' exists. Exiting ..."<<std::endl;
        exit(1);
    }
}

void checkPlTypeExists(std::string plType){
    if (!plTypeExists(plType)){
        std::cout<<"No payload type '"<<plType<<"' exists. Exiting..."<<std::endl;
        exit(1);
    }
}

std::string getPayloadListName(std::string gtName, std::string plType){
    nlohmann::json j = getPayloadLists(gtName);
    if (!j.contains(plType)){
        std::cout<<"global tag '"<<gtName<<"' does not have payload type '"<<plType<<"'. Exiting..."<<std::endl;
        exit(1);
    }
    return j[plType];
}



// Writing
void createGlobalTagType(std::string type){
    nlohmann::json j;
    j["name"] = type;
    curlwrapper::post(base_url + "gttype", j);
}

void createGlobalTagStatus(std::string status){
    nlohmann::json j;
    j["name"] = status;
    curlwrapper::post(base_url + "gtstatus", j);
}

void createGlobalTagObject(std::string name, std::string status, std::string type) {
    nlohmann::json j;
    j["status"] = status;
    //j["GlobalTagStatus"] = status;
    j["name"] = name;
    j["type"] = type;
    curlwrapper::post(base_url + "gt", j);
}

void createPayloadType(std::string type){
    nlohmann::json j;
    j["name"] = type;
    curlwrapper::post(base_url + "pt", j);
}

std::string createPayloadList(std::string type){
    nlohmann::json j;
    j["payload_type"] = type;
    nlohmann::json res = curlwrapper::post(base_url + "pl", j);
    return res["name"];
}

void attachPayloadList(std::string gtName, std::string plName){
    nlohmann::json j;
    j["payload_list"] = plName;
    j["global_tag"] = gtName;
    curlwrapper::put(base_url + "pl_attach", j);
}

void lockGlobalTag(std::string name){
    curlwrapper::put(base_url + "gt_change_status/" + name + "/locked");
}

void unlockGlobalTag(std::string name){
    curlwrapper::put(base_url + "gt_change_status/" + name + "/unlocked");
}

int createPayloadIOV(std::string plUrl, int majorIov, int minorIov){
    nlohmann::json j;
    j["payload_url"] = plUrl;
    j["major_iov"] = majorIov;
    j["minor_iov"] = minorIov;
    nlohmann::json res = curlwrapper::post(base_url + "piov", j);
    return res["id"];
}

int createPayloadIOV(std::string plUrl, int majorIov, int minorIov, int majorIovEnd, int minorIovEnd){
    nlohmann::json j;
    j["payload_url"] = plUrl;
    j["major_iov"] = majorIov;
    j["minor_iov"] = minorIov;
    j["major_iov_end"] = majorIovEnd;
    j["minor_iov_end"] = minorIovEnd;
    nlohmann::json res = curlwrapper::post(base_url + "piov", j);
    return res["id"];
}

void attachPayloadIOV(std::string plListName, int plIovId){
    nlohmann::json j;
    j["payload_list"] = plListName;
    j["piov_id"] = plIovId;
    nlohmann::json res = curlwrapper::put(base_url + "piov_attach", j);
}

}
