#pragma once

#include "base/Platform.hpp"
#include "server/EventLoop.hpp"

namespace cqnet {

class IEventLoopGroup
{
public:
    using IterateFunc = std::function<bool(int, EventLoop::Ptr)>;

    virtual void Register(EventLoop::Ptr el) = 0;
    virtual void Iterate(IterateFunc func) = 0;
    virtual size_t Len() = 0;
    virtual EventLoop::Ptr Next() = 0;
};

class RoundRobinEventLoopGroup
{
private:
    size_t size_;
    size_t next_loop_index_;
    std::vector<EventLoop::Ptr> event_loops;

public:
    void Register(EventLoop::Ptr el)
    {
        event_loops.push_back(std::move(el));
        size_++;
    }

    size_t Len()
    {
        return size_;
    }

    EventLoop::Ptr Next()
    {
        auto el = event_loops[next_loop_index_++];
        if (next_loop_index_ >= size_)
        {
            next_loop_index_ = 0;
        }
        return el;
    }

    void Iterate(IEventLoopGroup::IterateFunc func)
    {
        for (int i = 0; i < event_loops.size(); i++)
        {
            if (!func(i, event_loops[i]))
            {
                break;
            }
        }
    }
};

class LeastConnectionsEventLoopGroup
{
    std::vector<EventLoop*> event_loops;
};

class SourceAddrHashEventLoopGroup
{
    int size_;
    std::vector<EventLoop*> event_loops;
};

} // namespace cqnet
