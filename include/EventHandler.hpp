#pragma once

#include "base/Platform.hpp"
#include "NetConn.hpp"

namespace cqnet {

class CQNetServer;

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
    virtual std::tuple<char*, Action> OnOpened(NetConn::Ptr conn) = 0;
    virtual Action OnClosed(NetConn::Ptr conn) = 0;
    virtual void PreWrite() = 0;
    virtual void React() = 0;
    virtual void Tick() = 0;
};


class EventHandler
{
public:
    const Action OnInitComplete(const CQNetServer* svr)
    {
        return Action::None;
    }

    void OnShutdown(CQNetServer* svr) {}

    std::tuple<char*, Action> OnOpened(NetConn::Ptr conn)
    {
        return std::make_pair<char*, Action>(nullptr, Action::None);
    }

    Action OnClosed(NetConn::Ptr conn)
    {
        return Action::None;
    }

    void PreWrite() {}

    void React() {}

    void Tick() {}
};

} // namespace cqnet
