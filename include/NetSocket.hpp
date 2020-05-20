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
    friend TcpListenSocket;
};

class Socket : public FD
{
public:
    Socket(int fd)
        :FD(fd)
    {}

    int Read(char* send_ptr, int try_send)
    {
        return -1;
    }

    int Write(char* send_ptr, int try_send)
    {
        return -1;
    }
};

// tcp listener
// client & server side
class ConnSocket : public Socket
{
private:
    bool server_side_;

public:
    ConnSocket(int fd, bool server_side)
        : Socket(fd)
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

    void SetNoDelay()
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

    int Write(char* send_ptr, int try_send) override
    {
        return base::SocketSend(fd_, send_ptr, try_send);
    }

    int Read(char* recv_ptr, int try_recv) override
    {
        return base::SocketRecv(fd_, recv_ptr, try_recv);
    }
};

// tcp listener
class TcpListenSocket : public Socket
{
public:
    TcpListenSocket(int fd)
        : Socket(fd)
    {
    }

    ~TcpListenSocket()
    {
        base::SocketClose(fd_);
    }

    int Read(char* send_ptr, int try_send) override
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

    int Write(char* send_ptr, int try_send) override
    {
        return -1;
    }
};

} // namespace cqnet
