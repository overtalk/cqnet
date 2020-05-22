#pragma once

#include "NetSocket.hpp"

namespace cqnet {

// tcp listener
class TcpListenSocket : public FD
{
protected:
    TcpListenSocket(int fd)
        : FD(fd)
    {
    }

    ~TcpListenSocket() {}

public:
    using Ptr = std::shared_ptr<TcpListenSocket>;

    Ptr static Create(bool is_IPV6, const char* ip, int port, int back_num)
    {
        class make_shared_enabler : public TcpListenSocket
        {
        public:
            make_shared_enabler(int fd)
                : TcpListenSocket(fd)
            {
            }
        };

        int local_fd = cqnet::base::SocketTcpListen(is_IPV6, ip, port, back_num);

        return std::make_shared<make_shared_enabler>(local_fd);
    }

    // Read means accept a connection for tcp listener
    int Accept()
    {
        const auto conn_fd = base::SocketAccept(GetFD(), nullptr, nullptr);
        if (conn_fd == CQNET_INVALID_SOCKET)
        {
            // TODO: handle error
            // if (EINTR == BRYNET_ERRNO)
            // {
            //     throw EintrError();
            // }
            // else
            // {
            //     throw AcceptError(BRYNET_ERRNO);
            // }
        }

        return conn_fd;
    }
};

} // namespace cqnet
