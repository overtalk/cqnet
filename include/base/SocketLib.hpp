#pragma once

#include "base/Platform.hpp"

static int SocketCreate(int af, int type, int protocol)
{
    return ::socket(af, type, protocol);
}

static void SocketClose(int fd)
{
#ifdef CQNET_PLATFORM_WINDOWS
    ::closesocket(fd);
#elif defined CQNET_PLATFORM_LINUX || defined CQNET_PLATFORM_DARWIN
    ::close(fd);
#endif
}

static int SocketNodelay(int fd)
{
    const int flag = 1;
    return ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));
}

static bool SocketNonblock(int fd)
{
    int err;
    unsigned long ul = true;
#ifdef CQNET_PLATFORM_WINDOWS
    err = ioctlsocket(fd, FIONBIO, &ul);
#elif defined CQNET_PLATFORM_LINUX || defined CQNET_PLATFORM_DARWIN
    err = ioctl(fd, FIONBIO, &ul);
#endif

    return err != CQNET_SOCKET_ERROR;
}

static int SocketSetSendSize(int fd, int sd_size)
{
    return ::setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&sd_size, sizeof(sd_size));
}

static int SocketSetRecvSize(int fd, int rd_size)
{
    return ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&rd_size, sizeof(rd_size));
}

static int SocketCreate(int af, int type, int protocol)
{
    return ::socket(af, type, protocol);
}

static int SocketAccept(int listenSocket, struct sockaddr* addr, socklen_t* addrLen)
{
    return ::accept(listenSocket, addr, addrLen);
}

static int SocketRecv(int fd, char* recv_ptr, int try_revc_size)
{
    return ::recv(fd, recv_ptr, try_revc_size, 0);
}

static int SocketSend(int fd, char* send_ptr, int try_send_size)
{
    return ::send(fd, send_ptr, try_send_size, 0);
}
