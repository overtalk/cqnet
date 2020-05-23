#pragma once

#include "base/Platform.hpp"
#include "base/Buffer.hpp"
#include "NetAddr.hpp"

namespace cqnet {

enum class Action : uint8_t
{
    None = 0,
    Close = 1,
    ShutDown = 2
};

class CQNetServer;

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
    virtual bool AsyncWrite(char* data, size_t size) = 0;
    virtual bool Wakeup() = 0;
};

class IEventLoop
{
public:
    virtual void AddNewConn(int fd) = 0;
};

// IEventHandler defines the event handler interface
class IEventHandler
{
public:
    virtual Action OnInitComplete(const CQNetServer* svr) = 0;
    virtual void OnShutdown(CQNetServer* svr) = 0;
    virtual std::tuple<char*, Action> OnOpened(std::shared_ptr<INetConn> conn) = 0;
    virtual Action OnClosed(std::shared_ptr<INetConn> conn) = 0;
    virtual void PreWrite() = 0;
    virtual void React() = 0;
    virtual void Tick() = 0;
    // virtual Broadcast() = 0;
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

class ICodec
{
public:
    virtual char* Encode(std::shared_ptr<INetConn> conn, char* buf) = 0;
    virtual char* Decode(std::shared_ptr<INetConn> conn) = 0;
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
