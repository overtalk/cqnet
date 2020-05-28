#pragma once

#include "NetSocket.hpp"
#include "Interface.hpp"

namespace cqnet {

// tcp listener
class TcpListener
    : public Socket
    , public TcpListenSocket
{
private:
    // if get a new connection
    // use this to allocate the new connection
    std::shared_ptr<ILoadBalance> lb_;

protected:
    TcpListener(int fd, std::shared_ptr<ILoadBalance> lb)
        : TcpListenSocket(fd)
        , lb_(lb)
    {
    }

    ~TcpListener() {}

public:
    using Ptr = std::shared_ptr<TcpListener>;

    Ptr static Create(std::shared_ptr<ILoadBalance> lb, bool is_IPV6, const char* ip, int port, int back_num = 512)
    {
        class make_shared_enabler : public TcpListener
        {
        public:
            make_shared_enabler(int fd, std::shared_ptr<ILoadBalance> lb)
                : TcpListener(fd, std::move(lb))
            {
            }
        };

        int local_fd = cqnet::base::SocketTcpListen(is_IPV6, ip, port, back_num);
        return std::make_shared<make_shared_enabler>(local_fd, std::move(lb));
    }

    // Read means accept a connection for tcp listener
    // Accept is a better function name, but for common usage. call it read
    bool Read() override
    {
        std::cout << " tcp read " << std::endl;
        int new_conn_fd = AcceptTcpConn();
        std::cout << " 新连接的fd = " << new_conn_fd << std::endl;
        if (new_conn_fd == CQNET_INVALID_SOCKET)
        {
            return false;
        }

        std::shared_ptr<IEventLoop> el = lb_->Next();
        el->AddNewConn(new_conn_fd);

        return true;
    }

    bool Write() override
    {
        return false;
    }
};

} // namespace cqnet
