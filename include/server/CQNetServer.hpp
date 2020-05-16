#pragma once

#include "base/WaitGroup.hpp"
#include "netpoll/NetPoll.hpp"
#include "server/EventHandler.hpp"
#include "server/LoadBalance.hpp"

namespace cqnet {

class CQNetServer
{
private:
    base::WaitGroup wg_;
    EventHandler& event_handler_;
    EventLoopGroup* event_loop_group_;

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

    int GetListenerFD()
    {
        return 0;
    }
};

} // namespace cqnet