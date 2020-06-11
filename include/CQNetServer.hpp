#pragma once

#include "base/WaitGroup.hpp"
#include "netpoll/NetPoll.hpp"
#include "Interface.hpp"
#include "NetSocket.hpp"
#include "EventLoop.hpp"

namespace cqnet {

class CQNetServer
{
public:
    using ICodecPtr = std::shared_ptr<ICodec>;
    using ILoadBalancePtr = std::shared_ptr<ILoadBalance>;
    using IEventHandlerPtr = std::shared_ptr<IEventHandler>;
    using IEventLoopPtr = std::shared_ptr<IEventLoop>;

private:
    base::WaitGroup::Ptr wg_;
    IEventLoopPtr tcp_el_;
    // some common interface
    ICodecPtr codec_;
    ILoadBalancePtr lb_;
    IEventHandlerPtr event_handler_;

public:
    explicit CQNetServer(int thread_count, ICodecPtr c_ptr, ILoadBalancePtr lb_ptr, IEventHandlerPtr eh_ptr)
        : wg_(base::WaitGroup::Create())
        , codec_(std::move(c_ptr))
        , lb_(std::move(lb_ptr))
        , event_handler_(std::move(eh_ptr))
    {
        tcp_el_ = cqnet::EventLoop::Create(-1, codec_, event_handler_);
        lb_->Register(tcp_el_);

        for (int i = 0; i < thread_count; ++i)
        {
            auto el = cqnet::EventLoop::Create(i, codec_, event_handler_);
            std::cout << " register event loop " << std::endl;
            lb_->Register(std::move(el));
        }
    }

    ~CQNetServer() = default;

public:
    using Ptr = std::shared_ptr<CQNetServer>;

    Ptr static Create(int thread_count, ICodecPtr eh_ptr, ILoadBalancePtr lb_ptr, IEventHandlerPtr c_ptr)
    {
        class make_shared_enabler : public CQNetServer
        {
        public:
            make_shared_enabler(int thread_count, ICodecPtr eh_ptr, ILoadBalancePtr lb_ptr, IEventHandlerPtr c_ptr)
                : CQNetServer(thread_count, std::move(eh_ptr), std::move(lb_ptr), std::move(c_ptr))
            {
            }
        };

        return std::make_shared<CQNetServer>(thread_count, std::move(eh_ptr), std::move(lb_ptr), std::move(c_ptr));
    }

    bool AddTcp(bool is_IPV6, const char* ip, int port, int back_num = 512)
    {
        return tcp_el_->AddTcpListener(lb_, is_IPV6, ip, port, back_num);
    }

    void Run()
    {
        auto tf = [](int index, std::shared_ptr<IEventLoop> el_ptr) -> bool {
            std::cout << "xxx " << index << std::endl;
            std::thread t(std::bind(&IEventLoop::Run, el_ptr));
            t.detach();
            return true;
        };

        lb_->Iterate(std::move(tf));
    }
};

} // namespace cqnet