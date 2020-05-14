#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <assert.h>

#include "base/Platform.hpp"
#include "netpoll/NetPoll.hpp"

bool hanlder(int ev_fd, int16_t ev)
{
    switch (ev)
    {
    case EV_EOF:
        printf("Disconnect\n");
        close(ev_fd);
        // Socket is automatically removed from the kq by the kernel.
        break;

    default:

        break;
    }

    else if (fd == localFd)
    {
        // get a new connection
        struct sockaddr_storage addr;
        socklen_t socklen = sizeof(addr);
        int connfd = accept(fd, (struct sockaddr*)&addr, &socklen);
        assert(connfd != -1);

        // Listen on the new socket
        EV_SET(&evSet, connfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        kevent(kq, &evSet, 1, NULL, 0, NULL);
        printf("Got connection!\n");

        int flags = fcntl(connfd, F_GETFL, 0);
        assert(flags >= 0);
        fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

        // schedule to send the file when we can write (first chunk should happen immediately)
        EV_SET(&evSet, connfd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
        kevent(kq, &evSet, 1, NULL, 0, NULL);
    }
    else if (evList[i].filter == EVFILT_READ)
    {
        // Read from socket.
        char buf[1024];
        size_t bytes_read = recv(fd, buf, sizeof(buf), 0);
        printf("read %zu bytes\n", bytes_read);
    }
    else if (evList[i].filter == EVFILT_WRITE)
    {
        //                printf("Ok to write more!\n");

        off_t offset = (off_t)evList[i].udata;
        off_t len = 0; //evList[i].data;
        if (sendfile(junk, fd, offset, &len, NULL, 0) != 0)
        {
            //                    perror("sendfile");
            //                    printf("err %d\n", errno);

            if (errno == EAGAIN)
            {
                // schedule to send the rest of the file
                EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, (void*)(offset + len));
                kevent(kq, &evSet, 1, NULL, 0, NULL);
            }
        }
        bytes_written += len;
        printf("wrote %lld bytes, %lld total\n", len, bytes_written);
    }
}

int main()
{
    // Macos automatically binds both ipv4 and 6 when you do this.
    struct sockaddr_in6 addr = {};
    addr.sin6_len = sizeof(addr);
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any; //(struct in6_addr){}; // 0.0.0.0 / ::
    addr.sin6_port = htons(9999);

    int localFd = socket(addr.sin6_family, SOCK_STREAM, 0);
    assert(localFd != -1);

    int on = 1;
    setsockopt(localFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(localFd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        return 1;
    }
    assert(listen(localFd, 5) != -1);

    auto poller = cqnet::Poller::OpenPoller();

    if (!poller->AddRead(localFd))
    {
        std::cout << "failed to add read " << std::endl;
    }
}