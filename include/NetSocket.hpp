#pragma once

#include "base/Platform.hpp"
#include "base/SocketLib.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {

class ConnSocket;
class TcpListenSocket;

// FD defines the file desc
class FD : public base::NonCopyable
{
private:
    int fd_;

public:
    explicit FD(int fd)
        : fd_(fd)
    {
    }

    ~FD()
    {
        base::SocketClose(fd_);
    }

    int GetFD() const
    {
        return fd_;
    }

    friend ConnSocket;
    friend TcpListenSocket;
};

// tcp listener
// client & server side
class ConnSocket : public FD
{
private:
    bool server_side_;

public:
    ConnSocket(int fd, bool server_side)
        : FD(fd)
        , server_side_(server_side)
    {
    }

    ~ConnSocket() = default;

    bool IsServerSide() const
    {
        return server_side_;
    }

    bool SetNonblock()
    {
        return base::SocketNonblock(fd_);
    }

    void SetSendSize(int s_size)
    {
        base::SocketSetSendSize(fd_, s_size);
    }

    void SetReadSize(int r_size)
    {
        base::SocketSetRecvSize(fd_, r_size);
    }

    int WriteToSocket(char* send_ptr, int try_send)
    {
        return base::SocketSend(fd_, send_ptr, try_send);
    }

    int ReadFromSocket(char* recv_ptr, int try_recv)
    {
        return base::SocketRecv(fd_, recv_ptr, try_recv);
    }
};

class TcpListenSocket : public FD
{
public:
    explicit TcpListenSocket(int fd)
        : FD(fd)
    {
    }

    ~TcpListenSocket() = default;

    void SetNoDelay()
    {
        base::SocketNodelay(fd_);
    }

    int AcceptTcpConn()
    {
        const auto conn_fd = base::SocketAccept(fd_, nullptr, nullptr);
        if (conn_fd == CQNET_INVALID_SOCKET)
        {
            std::cout << "failed to get new connection" << std::endl;
            return -1;
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
