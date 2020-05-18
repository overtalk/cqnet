#pragma once

#include "base/Platform.hpp"
#include "Interface.hpp"

namespace cqnet {

class EventHandler : public IEventHandler
{
public:
    Action OnInitComplete(const CQNetServer* svr) override
    {
        return Action::None;
    }

    void OnShutdown(CQNetServer* svr) override {}

    std::tuple<char*, Action> OnOpened(std::shared_ptr<NetConn> conn) override
    {
        return std::make_pair<char*, Action>(nullptr, Action::None);
    }

    Action OnClosed(std::shared_ptr<NetConn> conn) override
    {
        return Action::None;
    }

    void PreWrite() override {}

    void React() override {}

    void Tick() override {}
};

} // namespace cqnet
