#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"
#include "netpoll/NetPoll.hpp"

namespace cqnet {

class Server;

class EventLoop : public base::Noncopyable
{
public:
    using Ptr = std::shared_ptr<EventLoop>;

    netpoll::Poller* GetPoller()
    {
        return poller;
    }

private:
    int index_;
    const Server* svr_;
    netpoll::Poller* poller;
};

} // namespace cqnet
