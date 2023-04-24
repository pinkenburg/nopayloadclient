#pragma once

#include <iostream>
#include <nlohmann/json.hpp>

#include <nopayloadclient/resthandler.hpp>
#include <nopayloadclient/payload.hpp>
#include <nopayloadclient/iov.hpp>
//#include <nopayloadclient/payloadiov.hpp>
#include <nopayloadclient/plhandler.hpp>
#include <nopayloadclient/config.hpp>
#include <nopayloadclient/exception.hpp>

#define NOPAYLOADCLIENT_TRY(...) {              \
    try {                       \
        __VA_ARGS__             \
    }                           \
    catch (BaseException &e) {  \
        return e.jsonify();     \
    }                           \
}                               \

namespace nopayloadclient {

using nlohmann::json;
using std::string;
using ll = long long;

class Client {
public:
    Client();
    Client(const string& gt_name);

    virtual json add(int a, int b) {
        std::cout << "Client::add(a=" << a << ", b=" << b << ")" << std::endl;
        return json {a+b};
    }

    // Configuration
    virtual json setGlobalTag(const string& name);
    virtual json getGlobalTag();
    virtual json override(const string& pl_type, const string& file_url);

    // Reading
    virtual json getUrlDict(ll major_iov, ll minor_iov);
    //virtual std::vector<PayloadIOV> getPayloadIOVs(ll major_iov, ll minor_iov);
    virtual json getPayloadIOVs(ll major_iov, ll minor_iov);

    // Writing
    virtual json createPayloadType(const string& pl_type);
    virtual json createGlobalTag();
    virtual json deleteGlobalTag();
    virtual json lockGlobalTag();
    virtual json unlockGlobalTag();
    virtual json cloneGlobalTag(const string& target);
    virtual json insertPayload(const string& pl_type, const string& file_url,
                       ll major_iov_start, ll minor_iov_start);
    virtual json insertPayload(const string& pl_type, const string& file_url,
                       ll major_iov_start, ll minor_iov_start,
                       ll major_iov_end, ll minor_iov_end);

    // Helper
    virtual json getSize();
    virtual json getPayloadTypes();
    virtual json getGlobalTags();
    virtual json checkConnection();
    virtual json getConfDict();
    virtual json clearCache();
    friend std::ostream& operator<<(std::ostream& os, const Client& c);
    template<typename T>
    json makeResp(T msg);



private:
    json config_;
    json override_dict_;
    string global_tag_;
    RESTHandler rest_handler_;
    PLHandler pl_handler_;

    // Writing
    virtual void prepareInsertIov(Payload &pl);
    virtual void insertIov(Payload& pl, IOV& iov);
    virtual void insertPayload(Payload &pl, IOV &iov);
    virtual void createNewPll(const string& pl_type);

    // Reading
    virtual bool gtExists(const string& name);
    virtual bool gtStatusExists(const string& name);
    virtual bool plTypeExists(const string& pl_type);
    virtual bool gtHasPlType(const string& pl_type);
    virtual void checkGtExists(const string& name);
    virtual void checkGtDoesNotExist(const string& name);
    virtual void checkGtStatusExists(const string& name);
    virtual void checkPlTypeExists(const string& name);
    //virtual json getUrlDict(const std::vector<PayloadIOV>& payload_iovs);
    virtual json getUrlDict(const json& payload_iovs);

    // Helper
    virtual bool objWithNameExists(const json& j, const string& name);
};

}
