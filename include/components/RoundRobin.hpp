#pragma once

#include "Interface.hpp"
#include "EventLoop.hpp"

namespace cqnet {

class RoundRobinLoadBalance : public ILoadBalance
{
private:
    size_t size_;
    size_t next_loop_index_;
    std::vector<std::shared_ptr<IEventLoop>> event_loops;

public:
    void Register(std::shared_ptr<IEventLoop> el) override
    {
        event_loops.push_back(std::move(el));
        size_++;
    }

    size_t Len() override
    {
        return size_;
    }

    std::shared_ptr<IEventLoop> Next() override
    {
        auto el = event_loops[next_loop_index_++];
        if (next_loop_index_ >= size_)
        {
            next_loop_index_ = 0;
        }
        return el;
    }

    void Iterate(IterateFunc func) override
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
