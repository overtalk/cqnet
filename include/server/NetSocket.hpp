#pragma once

#include "base/Platform.hpp"
#include "base/SocketLib.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {

class FD : public base::Noncopyable
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
        ::close(fd_);
    }

    int GetFD()
    {
        return fd_;
    }
};

// tcp listener
// client & server side
class TcpSocket
    : public FD
    , public base::Noncopyable

{
private:
    bool server_side_;

    class TcpSocketDeleter
    {
    public:
        void operator()(TcpSocket* ptr) const
        {
            delete ptr;
        }
    };

public:
    using Ptr = std::unique_ptr<TcpSocket, TcpSocketDeleter>;

    Ptr static Create(int fd, bool server_side)
    {
        class make_unique_enabler : public TcpSocket
        {
        public:
            make_unique_enabler(int fd, bool server_side)
                : TcpSocket(fd, server_side)
            {
            }
        };

        return Ptr(new make_unique_enabler(fd, server_side));
    }

    bool IsServerSide()
    {
        return server_side_;
    }

    bool SetNonblock()
    {
        return SocketNonblock(GetFD());
    }

    void SetNodelay()
    {
        SocketNodelay(GetFD());
    }

    void SetSendSize(int s_size)
    {
        SocketSetSendSize(GetFD(), s_size);
    }

    void SetReadSize(int r_size)
    {
        SocketSetRecvSize(GetFD(), r_size);
    }

protected:
    TcpSocket(int fd, bool server_side)
        : FD(fd)
        , server_side_(server_side)
    {
    }

    ~TcpSocket()
    {
        SocketClose(GetFD());
    }
};

// tcp listener
class ListenSocket
    : public base::Noncopyable
    , public FD
{

private:
    class ListenSocketDeleter
    {
    public:
        void operator()(ListenSocket* ptr) const
        {
            delete ptr;
        }
    };

public:
    using Ptr = std::unique_ptr<ListenSocket, ListenSocketDeleter>;
    static Ptr Create(int fd)
    {
        class make_unique_enabler : public ListenSocket
        {
        public:
            explicit make_unique_enabler(int fd)
                : ListenSocket(fd)
            {
            }
        };

        return Ptr(new make_unique_enabler(fd));
    }

    TcpSocket::Ptr Accept()
    {
        const auto clientFD = SocketAccept(GetFD(), nullptr, nullptr);
        if (clientFD == CQNET_INVALID_SOCKET)
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

        return TcpSocket::Create(clientFD, true);
    }

protected:
    explicit ListenSocket(int fd)
        : FD(fd)
    {
    }

    ~ListenSocket()
    {
        SocketClose(GetFD());
    }
};

} // namespace cqnet
