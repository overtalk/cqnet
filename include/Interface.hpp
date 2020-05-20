#pragma once

#include "base/Platform.hpp"

namespace cqnet {

    class NetConn;
    class EventLoop;
    class CQNetServer;
    using IterateFunc = std::function<bool(int, std::shared_ptr<EventLoop>)>;

    enum class Action : uint8_t
    {
        None = 0,
        Close = 1,
        ShutDown = 2
    };

    // IEventHandler defines the event handler interface
    class IEventHandler
    {
    public:
        virtual Action OnInitComplete(const CQNetServer* svr) = 0;
        virtual void OnShutdown(CQNetServer* svr) = 0;
        virtual std::tuple<char*, Action> OnOpened(std::shared_ptr<NetConn> conn) = 0;
        virtual Action OnClosed(std::shared_ptr<NetConn> conn) = 0;
        virtual void PreWrite() = 0;
        virtual void React() = 0;
        virtual void Tick() = 0;
    };

    class ILoadBalance {
    public:
        virtual void Register(std::shared_ptr <EventLoop> el) = 0;
        virtual void Iterate(IterateFunc func) = 0;
        virtual size_t Len() = 0;
        virtual std::shared_ptr <EventLoop> Next() = 0;
    };

    class ICodec {
    public:
        virtual char *Encode(std::shared_ptr<NetConn> conn, char *buf) = 0;
        virtual char *Decode(std::shared_ptr<NetConn> conn) = 0;
    };

} // namespace cqnet
