#pragma once

#include "base/Platform.hpp"
#include "base/SocketLib.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {

class ConnSocket;
class ListenSocket;

// FD defines the file desc
class FD : public base::NonCopyable
{
private:
    int fd_;

public:
    FD(int fd)
        : fd_(fd)
    {
    }

    ~FD()
    {
        base::SocketClose(fd_);
    }

    int GetFD()
    {
        return fd_;
    }

    friend ConnSocket;
    friend ListenSocket;
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

    ~ConnSocket()
    {
        base::SocketClose(fd_);
    }

    bool IsServerSide()
    {
        return server_side_;
    }

    bool SetNonblock()
    {
        return base::SocketNonblock(fd_);
    }

    void SetNodelay()
    {
        base::SocketNodelay(fd_);
    }

    void SetSendSize(int s_size)
    {
        base::SocketSetSendSize(fd_, s_size);
    }

    void SetReadSize(int r_size)
    {
        base::SocketSetRecvSize(fd_, r_size);
    }

    int SendToSocket(char* send_ptr, int try_send)
    {
        return base::SocketSend(fd_, send_ptr, try_send);
    }

    int RecvFromSocket(char* recv_ptr, int try_recv)
    {
        return base::SocketRecv(fd_, recv_ptr, try_recv);
    }
};

// tcp listener
class ListenSocket : public FD
{

public:
    explicit ListenSocket(int fd)
        : FD(fd)
    {
    }

    ~ListenSocket()
    {
        base::SocketClose(fd_);
    }

    int Accept()
    {
        const auto conn_fd = base::SocketAccept(fd_, nullptr, nullptr);
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
