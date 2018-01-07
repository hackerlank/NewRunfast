#ifndef GAMEMESSAGEMGR_H
#define GAMEMESSAGEMGR_H

#include <boost/asio.hpp>
#include <assistx2/tcphandler_wrapper.h>
#include <assistx2/stream2.h>

//template<class T>
//class Singleton
//{
//private:
//    Singleton(){}   //构造函数是私有的
//    Singleton(const Singleton &);
//    Singleton & operator=(const Singleton &);
//    virtual ~Singleton(){}
//    static T *pInstance_;
//public:
//    static T * GetInstance()
//    {
//        if(pInstance_ == NULL)  //判断是否第一次调用
//            pInstance_ = new T;
//        return pInstance_;
//    }
//};

class RunFastGameMgr;
class MatchProxy;
class MatchHall;
class HandleObj : public boost::shared_ptr<HandleObj>
{
public:
    HandleObj();
    virtual ~HandleObj();

    virtual void Initialize();
    virtual std::int32_t OnMessage(assistx2::TcpHandler * socket, boost::shared_ptr<assistx2::NativeStream > native_stream) {return 0;}
    virtual std::int32_t OnConnect(assistx2::TcpHandler * handler, assistx2::ErrorCode err) {return 0;}
    virtual std::int32_t OnClose(assistx2::TcpHandler * handler, assistx2::ErrorCode err) {return 0;}

    virtual int OnProxyMessage(assistx2::TcpHandler * socket, boost::shared_ptr< assistx2::NativeStream >  packet) {return 0;}
    virtual int OnProxyConnect(assistx2::TcpHandler * handler) {return 0;}
    virtual int OnProxyClose(assistx2::TcpHandler * handler) {return 0;}

    const boost::asio::io_service& service() const {return service_;}
    const boost::asio::deadline_timer& timer() const {return timer_;}
    const boost::asio::deadline_timer& event_timer() const {return event_timer_;}
    const assistx2::TcpHanlderWrapper& gatewayconnector() const {return gatewayconnector_;}
    const assistx2::TcpHanlderWrapper& proxy() const {return proxy_;}

protected:
    boost::asio::io_service service_;
    boost::asio::deadline_timer timer_;
    boost::asio::deadline_timer event_timer_;
    assistx2::TcpHanlderWrapper gatewayconnector_;
    assistx2::TcpHanlderWrapper proxy_;
};

#endif // GAMEMESSAGEMGR_H
