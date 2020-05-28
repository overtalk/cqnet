#pragma once

#include "base/Platform.hpp"
#include "Interface.hpp"

namespace cqnet {

class EventHandler : public IEventHandler
{
public:
    Action OnInitComplete(const CQNetServer* svr) override
    {
        std::cout << "OnInitComplete" << std::endl;
        return Action::None;
    }

    void OnShutdown(CQNetServer* svr) override
    {
        std::cout << "OnShutdown" << std::endl;
    }

    std::tuple<char*, Action> OnOpened(std::shared_ptr<INetConn> conn) override
    {
        std::cout << "OnOpened" << std::endl;
        return std::make_pair<char*, Action>(nullptr, Action::None);
    }

    Action OnClosed(std::shared_ptr<INetConn> conn) override
    {
        std::cout << "OnClosed" << std::endl;
        return Action::None;
    }

    void PreWrite() override
    {
        std::cout << "PreWrite" << std::endl;
    }

    void React(base::SDS::Ptr sds) override
    {
        char* data = sds->GetData();
        int size = sds->GetSize();
        std::cout << "In React Func :" << size << " " << data << std::endl;
    }

    void Tick() override
    {
        std::cout << "Tick" << std::endl;
    }
};

} // namespace cqnet
