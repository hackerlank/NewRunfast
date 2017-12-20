#pragma once

#include <memory>
#include <string>
#include <functional>

using ResponseCallback = std::function<void(const std::string& response)>;

class HttpClientImpl;
class HttpClient
{
public:
    HttpClient();
    virtual~HttpClient();
    std::int32_t Post_Json(const std::string& url, const std::string& post, ResponseCallback callback);
    std::int32_t Post(const std::string& url,const std::string& post, ResponseCallback callback);
    std::int32_t Get(const std::string& url, ResponseCallback callback);
    std::int32_t Posts(const std::string& url, const std::string& post, ResponseCallback callback, const std::string& capath = "");
    std::int32_t Gets(const std::string& url, ResponseCallback callback,const std::string& capath = "");
private:
    std::unique_ptr<HttpClientImpl> pImpl_;
};

