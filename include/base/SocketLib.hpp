#pragma once

#include "base/Platform.hpp"

namespace cqnet {
namespace base {

// @brief: SocketCreate func is to create a socket
// @param: domain : 指定通信协议族。常用的协议族有AF_INET、AF_UNIX等，对于TCP协议，该字段应为AF_INET（ipv4）或AF_INET6（ipv6）。
// @param: type : 指定socket类型。常用的socket类型有SOCK_STREAM、SOCK_DGRAM等。SOCK_STREAM针对于面向连接的TCP服务应用。SOCK_DGRAM对应于无连接的UDP服务应用。
// @param: protocol : 指定socket所使用的协议，一般我们平常都指定为0，使用type中的默认协议。严格意义上，IPPROTO_TCP（值为6）代表TCP协议。
// @ret: socket fd
static int SocketCreate(int domain, int type, int protocol)
{
    return ::socket(domain, type, protocol);
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

// @brief: SocketListen func is to create a tcp listen socket with `bind & listen systemcall` called
// @param: back_num : 保存等待处理的请求的队列的最大长度
// @ret: tcp listen socket fd
static int SocketTcpListen(bool isIPV6, const char* ip, int port, int back_num)
{
    // build listen address
    struct sockaddr_in ip4Addr = sockaddr_in();
    struct sockaddr_in6 ip6Addr = sockaddr_in6();
    struct sockaddr_in* paddr = &ip4Addr;
    int addr_len = sizeof(ip4Addr);

    const auto listen_socket_fd =
        isIPV6 ? SocketCreate(AF_INET6, SOCK_STREAM, 0) : SocketCreate(AF_INET, SOCK_STREAM, 0);
    if (listen_socket_fd == CQNET_INVALID_SOCKET)
    {
        return CQNET_INVALID_SOCKET;
    }

    bool pton_result = false;
    if (isIPV6)
    {
        ip6Addr.sin6_family = AF_INET6;
        ip6Addr.sin6_port = htons(port);
        pton_result = inet_pton(AF_INET6, ip, &ip6Addr.sin6_addr) > 0;
        paddr = (struct sockaddr_in*)&ip6Addr;
        addr_len = sizeof(ip6Addr);
    }
    else
    {
        ip4Addr.sin_family = AF_INET;
        ip4Addr.sin_port = htons(port);
        ip4Addr.sin_addr.s_addr = INADDR_ANY;
        pton_result = inet_pton(AF_INET, ip, &ip4Addr.sin_addr) > 0;
    }

    const int reuseaddr_value = 1;
    if (!pton_result ||
        ::setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr_value, sizeof(int)) < 0)
    {
        SocketClose(listen_socket_fd);
        return CQNET_INVALID_SOCKET;
    }

    const int bind_ret = ::bind(listen_socket_fd, (struct sockaddr*)paddr, addr_len);
    if (bind_ret == CQNET_SOCKET_ERROR || listen(listen_socket_fd, back_num) == CQNET_SOCKET_ERROR)
    {
        SocketClose(listen_socket_fd);
        return CQNET_INVALID_SOCKET;
    }

    return listen_socket_fd;
}

} // namespace base
} // namespace cqnet
