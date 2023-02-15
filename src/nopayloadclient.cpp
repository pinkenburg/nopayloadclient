#include <nopayloadclient/nopayloadclient.hpp>

namespace nopayloadclient {

Client::Client() {
    config_ = getDict();
    rest_handler_ = RESTHandler(config_);
    pl_handler_ = PLHandler(config_);
}

Client::Client(std::string gt_name) : Client() {
    rest_handler_.setGlobalTag(gt_name);
}

// Configuring
json Client::setGlobalTag(std::string name) {
    TRY (
        rest_handler_.setGlobalTag(name);
        return makeResp("successfully changed global tag to " + name);
    )
}

json Client::getGlobalTag() {
    TRY (
        return makeResp(rest_handler_.getGlobalTag());
    )
}

// Reading
json Client::getUrlDict(ll major_iov, ll minor_iov){
    TRY(
        checkGtExists();
        Moment mom {major_iov, minor_iov};
        json url_dict = getSuffixDict(mom);
        prependReadDirs(url_dict);
        return makeResp(url_dict);
    )
}

// Writing
json Client::createGlobalTag() {
    TRY(
        if (!gtStatusExists("unlocked")){
            rest_handler_.createGlobalTagStatus("unlocked");
        }
        rest_handler_.createGlobalTagObject("unlocked");
        return makeResp("successfully created global tag");
    )
}

json Client::deleteGlobalTag() {
    TRY(
        rest_handler_.deleteGlobalTag();
        return makeResp("successfully deleted global tag");
    )
}

json Client::lockGlobalTag() {
    TRY(
        if (!gtStatusExists("locked")){
            rest_handler_.createGlobalTagStatus("locked");
        }
        rest_handler_.lockGlobalTag();
        return makeResp("successfully locked global tag");
    )
}

json Client::unlockGlobalTag() {
    TRY(
        if (!gtStatusExists("unlocked")){
            rest_handler_.createGlobalTagStatus("unlocked");
        }
        rest_handler_.unlockGlobalTag();
        return makeResp("successfully unlocked global tag");
    )
}

json Client::createPayloadType(std::string name) {
    TRY(
        rest_handler_.createPayloadType(name);
        return makeResp("successfully created payload type");
    )
}

json Client::insertPayload(std::string pl_type, std::string file_url,
                           ll major_iov_start, ll minor_iov_start) {
    TRY(
        Payload pl {file_url, pl_type};
        IOV iov {major_iov_start, minor_iov_start};
        insertPayload(pl, iov);
        return makeResp("successfully inserted payload");
    )
}

json Client::insertPayload(std::string pl_type, std::string file_url,
                           ll major_iov_start, ll minor_iov_start,
                           ll major_iov_end, ll minor_iov_end) {
    TRY(
        Payload pl {file_url, pl_type};
        IOV iov {major_iov_start, minor_iov_start, major_iov_end, minor_iov_end};
        insertPayload(pl, iov);
        return makeResp("successfully inserted payload");
    )
}

// Helper
json Client::getSize(){
    TRY(
        int n_iov_attached = 0;
        int n_gt = 0;
        for (auto gt : rest_handler_.getGlobalTags()){
            n_iov_attached += int(gt["payload_iov_count"]);
            n_gt += 1;
        }
        json j = {{"n_global_tag", n_gt},
                  {"n_iov_attached", n_iov_attached},
                  {"n_pt", getPayloadTypes().size()}};
        return makeResp(j);
    )
}

json Client::getPayloadTypes(){
    TRY(
        return makeResp(rest_handler_.getPayloadTypes());
    )
}

json Client::getGlobalTags(){
    TRY(
        return makeResp(rest_handler_.getGlobalTags());
    )
}

json Client::checkConnection(){
    TRY(
        rest_handler_.getGlobalTags();
        return makeResp("connection is good");
    )
}

json Client::getConfDict(){
    TRY(
        return makeResp(config_);
    )
}

std::ostream& operator<<(std::ostream& os, const Client& c) {
    os << "Client instance with following attributes:" << std::endl;
    os << "config = " << c.config_ << std::endl;
    return os;
}

// Private
template<typename T>
json Client::makeResp(T msg) {
    return {{"code", 0}, {"msg", msg}};
}

void Client::insertPayload(Payload &pl, IOV &iov) {
    prepareInsertIov(pl);
    pl_handler_.prepareUploadFile(pl);
    insertIov(pl, iov);
    pl_handler_.uploadFile(pl);
}

void Client::prepareInsertIov(Payload &pl) {
    checkGtExists();
    checkPlTypeExists(pl.type);
    if (!gtHasPlType(pl.type)) {
        std::cout << "gt " << rest_handler_.getGlobalTag() << " has no pl type " << pl.type << std::endl;
        std::cout << "attempting to attach it..." << std::endl;
        createNewPll(pl.type);
    }
}

bool Client::gtStatusExists(std::string name){
    json j = rest_handler_.getGlobalTagStatuses();
    return objWithNameExists(j, name);
}

bool Client::gtExists(){
    json j = rest_handler_.getGlobalTags();
    return objWithNameExists(j, rest_handler_.getGlobalTag());
}

bool Client::plTypeExists(std::string pl_type){
    json j = rest_handler_.getPayloadTypes();
    return objWithNameExists(j, pl_type);
}

void Client::checkGtStatusExists(std::string name){
    if (!gtStatusExists(name)){
        std::string msg = "no global tag status with name '"+name+"' exists";
        throw BaseException(msg);
    }
}

void Client::checkGtExists(){
    if (!gtExists()){
        std::string msg = "no global tag with name '"+rest_handler_.getGlobalTag()+"' exists";
        throw BaseException(msg);
    }
}

void Client::checkPlTypeExists(std::string pl_type){
    if (!plTypeExists(pl_type)){
        std::string msg = "no payload type with name '"+pl_type+"' exists";
        throw BaseException(msg);
    }
}

bool Client::gtHasPlType(std::string pl_type){
    json j = rest_handler_.getPayloadLists();
    return (j.contains(pl_type));
}

void Client::createNewPll(std::string pl_type){
    std::string pll_name = rest_handler_.createPayloadList(pl_type);
    rest_handler_.attachPayloadList(pll_name);
}

void Client::insertIov(Payload& pl, IOV& iov) {
    std::string pll_name = rest_handler_.getPayloadLists()[pl.type];
    ll piov_id = rest_handler_.createPayloadIOV(pl, iov);
    rest_handler_.attachPayloadIOV(pll_name, piov_id);
}

json Client::getSuffixDict(Moment& mom) {
    json suffix_dict;
    json j = rest_handler_.getPayloadIOVs(mom);
    for (const json& el : j){
        std::string pl_type = el["payload_type"];
        std::string url = el["payload_iov"][0]["payload_url"];
        suffix_dict[pl_type] = url;
    }
    return suffix_dict;
}

nlohmann::json Client::prependReadDirs(json& suffix_dict) {
    json prepended_dict;
    for (auto& obj : suffix_dict.items()) {
        prepended_dict[obj.key()] = std::vector<std::string> {};
        for (std::string rd : config_["read_dir_list"]) {
            prepended_dict[obj.key()].push_back(rd + (std::string)obj.value());
        }
    }
    return prepended_dict;
}

bool Client::objWithNameExists(const json& j, std::string name) {
    for (const auto& obj: j){
        if (obj["name"] == name) return true;
    }
    return false;
}

}
