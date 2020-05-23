#pragma once

#include "base/WaitGroup.hpp"
#include "netpoll/NetPoll.hpp"
#include "Interface.hpp"
#include "NetSocket.hpp"
#include "EventLoop.hpp"

namespace cqnet {

class CQNetServer
{
private:
    base::WaitGroup::Ptr wg_;
    TcpListener::Ptr tcp_listener_;
    // some common interface
    std::shared_ptr<ICodec> codec_;
    std::shared_ptr<ILoadBalance> lb_;
    std::shared_ptr<IEventHandler> event_handler_;

public:
    CQNetServer()
        : wg_(base::WaitGroup::Create())
    {
    }

    ~CQNetServer() {}

public:
    using Ptr = std::shared_ptr<CQNetServer>;

    Ptr static Create()
    {
        class make_shared_enabler : public CQNetServer
        {
        public:
            make_shared_enabler()
                : CQNetServer()
            {
            }
        };

        return std::make_shared<CQNetServer>();
    }

    void SetIEventHandler(std::shared_ptr<IEventHandler> ptr_)
    {
        event_handler_ = std::move(ptr_);
    }

    void SetILoadBalance(std::shared_ptr<ILoadBalance> ptr_)
    {
        lb_ = std::move(ptr_);
    }

    void SetICodec(std::shared_ptr<ICodec> ptr_)
    {
        codec_ = std::move(ptr_);
    }

    void SetTcpServer(bool is_IPV6, const char* ip, int port, int back_num = 512)
    {
        tcp_listener_ = std::move(TcpListener::Create(nullptr, is_IPV6, ip, port, back_num));
    }
};

} // namespace cqnet