#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include "httpclient.h"
#include <curl/curl.h>

class HttpClientImpl
{
public:
    HttpClientImpl();
    ~HttpClientImpl();
    void set_share_handle(CURL* curl_handle);
public:
    ResponseCallback rcallbackfunc_;
};

HttpClient::HttpClient():
   pImpl_(new HttpClientImpl)
{
    //多线程调用时 需在主线程调用curl_global_init初始化
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpClient::~HttpClient()
{
    curl_global_cleanup();
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    HttpClientImpl* pImpl = dynamic_cast<HttpClientImpl*>((HttpClientImpl*)lpVoid);
    if (nullptr == pImpl || nullptr == buffer)
    {
        return -1;
    }

    std::string str;
    char* pData = (char*)buffer;
    str.append(pData, size * nmemb);

    if (pImpl->rcallbackfunc_ != nullptr)
    {
        pImpl->rcallbackfunc_(str);
    }
    
    return nmemb;
}

std::int32_t HttpClient::Post_Json(const std::string& url, const std::string& post, ResponseCallback callback)
{
    CURLcode res;
    CURL* curl = nullptr;
    pImpl_->set_share_handle(curl);
    curl = curl_easy_init();
    if (nullptr == curl)
    {
        return CURLE_FAILED_INIT;
    }

    pImpl_->rcallbackfunc_ = callback;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, nullptr);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pImpl_.get());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
    curl_slist *plist = curl_slist_append(NULL,"Content-Type:application/json");
    plist = curl_slist_append(plist, "Content-Encoding: UTF-8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        DLOG(ERROR) << "Post_Json: failed err:=" << res << ",post:=" << post;
    }
    DLOG(INFO) << "res = " << res;

    curl_slist_free_all(plist);
    curl_easy_cleanup(curl);

    return res;
}

std::int32_t HttpClient::Post(const std::string& url, const std::string& post, ResponseCallback callback)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
    {
        return CURLE_FAILED_INIT;
    }

    pImpl_->rcallbackfunc_ = callback;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, nullptr);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pImpl_.get());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return res;
}

std::int32_t HttpClient::Get(const std::string& url, ResponseCallback callback)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
    {
        return CURLE_FAILED_INIT;
    }

    pImpl_->rcallbackfunc_ = callback;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pImpl_.get());
    /**
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

std::int32_t HttpClient::Posts(const std::string& url, const std::string& post, ResponseCallback callback, const std::string& capath)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
    {
        return CURLE_FAILED_INIT;
    }

    pImpl_->rcallbackfunc_ = callback;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pImpl_.get());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    if (capath.empty())
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    }
    else
    {
        //缺省情况就是PEM，所以无需设置，另外支持DER  
        //curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(curl, CURLOPT_CAINFO, capath.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

std::int32_t HttpClient::Gets(const std::string& url, ResponseCallback callback, const std::string& capath)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
    {
        return CURLE_FAILED_INIT;
    }

    pImpl_->rcallbackfunc_ = callback;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pImpl_.get());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    if (capath.empty())
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    }
    else
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(curl, CURLOPT_CAINFO, capath.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

HttpClientImpl::HttpClientImpl()
{
}

HttpClientImpl::~HttpClientImpl()
{
}

void HttpClientImpl::set_share_handle(CURL* curl_handle)
{
    static CURLSH* share_handle = nullptr;
    if (share_handle == nullptr)
    {
        share_handle = curl_share_init();
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    }
    curl_easy_setopt(curl_handle, CURLOPT_SHARE, share_handle);
    curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 60 * 5);
}
