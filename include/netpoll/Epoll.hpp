#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {
namespace netpoll {

class Epoll : public base::NonCopyable
{
private:
    int epoll_fd_;

public:
    Epoll()
    {
        int epoll_fd = epoll_create1();
        if (epoll_fd == -1)
        {
            // TODO: handle error
            return;
        }

        epoll_fd_ = epoll_fd;
    }
};

} // namespace netpoll
} // namespace cqnet