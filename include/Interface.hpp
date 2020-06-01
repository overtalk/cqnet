#pragma once

#include "base/Platform.hpp"
#include "base/Buffer.hpp"
#include "base/SDS.hpp"
#include "NetAddr.hpp"

namespace cqnet {

enum class Action : uint8_t
{
    None = 0,
    Close = 1,
    ShutDown = 2
};

class CQNetServer;
class IEventLoop;

class INetConn
{
public:
    virtual void Context() = 0;
    virtual void SetContext() = 0;
    virtual NetAddr::Ptr LocalAddr() = 0;
    virtual NetAddr::Ptr RemoteAddr() = 0;
    virtual std::tuple<char*, size_t> ReadBuffer() = 0;
    virtual bool ShiftN(size_t size) = 0;
    virtual void ResetBuffer() = 0;
    virtual bool AsyncWrite(base::SDS::Ptr sds) = 0;
    virtual bool Wakeup() = 0;
};

class ILoadBalance
{
public:
    using IterateFunc = std::function<bool(int, std::shared_ptr<IEventLoop>)>;
    virtual void Register(std::shared_ptr<IEventLoop> el) = 0;
    virtual void Iterate(IterateFunc func) = 0;
    virtual size_t Len() = 0;
    virtual std::shared_ptr<IEventLoop> Next() = 0;
};

class IEventLoop
{
public:
    virtual void Run() = 0;
    virtual void AddNewConn(int fd) = 0;
    virtual bool AddTcpListener(
        std::shared_ptr<ILoadBalance> lb, bool is_IPV6, const char* ip, int port, int back_num) = 0;
};

// IEventHandler defines the event handler interface
class IEventHandler
{
public:
    using Ret = std::tuple<base::SDS::Ptr, Action>;

    virtual Action OnInitComplete(const CQNetServer* svr) = 0;
    virtual void OnShutdown(CQNetServer* svr) = 0;
    virtual Ret OnOpened(INetConn* conn) = 0;
    virtual Action OnClosed(INetConn* conn) = 0;
    virtual void PreWrite() = 0;
    virtual Ret React(base::SDS::Ptr sds) = 0;
    virtual void Tick() = 0;
    virtual void Broadcast() = 0;
};

class ICodec
{
public:
    virtual base::SDS::Ptr Encode(INetConn* conn, base::SDS::Ptr sds) = 0;
    virtual base::SDS::Ptr Decode(INetConn* conn) = 0;
};

// Socket defines the thing to store into epoll/kqueue
// if there is something happen in the fd, call function blow to do
class Socket
{
public:
    virtual bool Read() = 0;
    virtual bool Write() = 0;
};

} // namespace cqnet
