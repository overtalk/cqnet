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

    IEventHandler::Ret OnOpened(INetConn* conn) override
    {
        std::cout << "OnOpenedSDS" << std::endl;
        auto temp = "qqq";
        return std::make_pair<base::SDS::Ptr, Action>(base::SDS::Create(temp, strlen(temp)), Action::None);
    }

    Action OnClosed(INetConn* conn) override
    {
        std::cout << "OnClosed" << std::endl;
        return Action::None;
    }

    void PreWrite() override
    {
        std::cout << "PreWrite" << std::endl;
    }

    IEventHandler::Ret React(base::SDS::Ptr sds) override
    {
        char* data = sds->GetData();
        int size = sds->GetSize();
        std::cout << "In React Func :" << size << " " << data;
        return std::make_pair<base::SDS::Ptr, Action>(base::SDS::Create(data, size), Action::None);
    }

    void Tick() override
    {
        std::cout << "Tick" << std::endl;
    }

    void Broadcast() override
    {
        std::cout << "Broadcast" << std::endl;
    }
};

} // namespace cqnet
