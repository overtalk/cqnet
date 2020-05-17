#pragma once

#include "base/WaitGroup.hpp"
#include "netpoll/NetPoll.hpp"
#include "server/EventHandler.hpp"
#include "server/LoadBalance.hpp"
#include "server/NetSocket.hpp"

namespace cqnet {

class CQNetServer : public ListenSocket
{
private:
    base::WaitGroup wg_;
    EventHandler& event_handler_;
    IEventLoopGroup* event_loop_group_;

public:
    template<class T>
    CQNetServer()
    {
        if (!std::is_base_of<EventHandler, T>::value)
        {
            // TODO: error handler;
        }

        event_handler_ = T();
    }

    ~CQNetServer();
};

} // namespace cqnet