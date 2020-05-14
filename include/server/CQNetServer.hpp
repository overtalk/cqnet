#pragma once

#include "base/WaitGroup.hpp"
#include "netpoll/NetPoll.hpp"
#include "server/EventHandler.hpp"

namespace cqnet {

class CQNetServer
{
private:
    base::WaitGroup wg_;
    const EventHandler& event_handler_;

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