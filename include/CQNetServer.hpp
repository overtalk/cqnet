#pragma once

#include "base/WaitGroup.hpp"
#include "netpoll/NetPoll.hpp"
#include "Interface.hpp"
#include "NetSocket.hpp"

namespace cqnet {

class CQNetServer : public TcpListenSocket
{
private:
    base::WaitGroup wg_;
    std::shared_ptr<IEventHandler> event_handler_;
    std::shared_ptr<ILoadBalance> event_loop_group_;

public:
    using Ptr = std::shared_ptr<CQNetServer>;

    template<class T>
    CQNetServer()
    {
        if (!std::is_base_of<IEventHandler, T>::value)
        {
            // TODO: error handler;
        }

        event_handler_ = T();
    }

    ~CQNetServer();
};

} // namespace cqnet