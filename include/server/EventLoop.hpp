#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {

class Server;

class EventLoop : public base::Noncopyable
{
public:
    using Ptr = std::shared_ptr<EventLoop>;

private:
    int index_;
    const Server* svr_;

public:
    EventLoop(/* args */);
    ~EventLoop();
};

EventLoop::EventLoop(/* args */) {}

EventLoop::~EventLoop() {}

} // namespace cqnet
