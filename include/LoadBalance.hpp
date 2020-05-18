#pragma once

#include "base/Platform.hpp"
#include "EventLoop.hpp"

namespace cqnet {

using IterateFunc = std::function<bool(int, std::shared_ptr<EventLoop>)>;

class IEventLoopGroup
{
public:
    virtual void Register(std::shared_ptr<EventLoop> el) = 0;
    virtual void Iterate(IterateFunc func) = 0;
    virtual size_t Len() = 0;
    virtual std::shared_ptr<EventLoop> Next() = 0;
};

class RoundRobinEventLoopGroup
{
private:
    size_t size_;
    size_t next_loop_index_;
    std::vector<std::shared_ptr<EventLoop>> event_loops;

public:
    void Register(std::shared_ptr<EventLoop> el)
    {
        event_loops.push_back(std::move(el));
        size_++;
    }

    size_t Len()
    {
        return size_;
    }

    std::shared_ptr<EventLoop> Next()
    {
        auto el = event_loops[next_loop_index_++];
        if (next_loop_index_ >= size_)
        {
            next_loop_index_ = 0;
        }
        return el;
    }

    void Iterate(IterateFunc func)
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

} // namespace cqnet
