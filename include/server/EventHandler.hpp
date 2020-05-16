#pragma once

#include "base/Platform.hpp"

namespace cqnet {

class NetConn;
class Server;

enum class Action : uint8_t
{
    None = 0,
    Close = 1,
    ShutDown = 2
};

class EventHandler
{
public:
    const Action OnInitComplete(const Server* svr)
    {
        return Action::None;
    }

    void OnShutdown(Server* svr) {}

    std::tuple<char*, Action> OnOpened(NetConn* conn)
    {
        return std::make_pair<char*, Action>(nullptr, Action::None);
    }

    Action OnClosed(NetConn* conn)
    {
        return Action::None;
    }

    void PreWrite() {}

    void React() {}

    void Tick() {}
};

} // namespace cqnet
