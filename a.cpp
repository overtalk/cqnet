#include <sys/socket.h>
#include <sys/un.h>
#include <sys/event.h>
#include <netdb.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>

#include "base/SocketLib.hpp"
#include "netpoll/NetPoll.hpp"

int main(int argc, const char* argv[])
{
    // create listen fd
    int localFd = cqnet::base::SocketTcpListen(false, "127.0.0.1", 9999, 512);
    std::cout << localFd << std::endl;

    int kq = kqueue();

    // 向kqueue的增加listen socket 的 read事件
    struct kevent evSet;
    EV_SET(&evSet, localFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    assert(-1 != kevent(kq, &evSet, 1, NULL, 0, NULL));

    int junk = open("some.big.file", O_RDONLY);

    uint64_t bytes_written = 0;

    struct kevent evList[32];
    while (1)
    {
        // returns number of events
        int nev = kevent(kq, NULL, 0, evList, 32, NULL);
        //        printf("kqueue got %d events\n", nev);

        for (int i = 0; i < nev; i++)
        {
            int fd = (int)evList[i].ident;

            if (evList[i].flags & EV_EOF)
            {
                printf("Disconnect\n");
                close(fd);
                // Socket is automatically removed from the kq by the kernel.
            }
            else if (fd == localFd)
            {
                // socket fd， 有新的链接进入
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
                // 读取的系统调用
                size_t bytes_read = recv(fd, buf, sizeof(buf), 0);
                printf("read %zu bytes\n", bytes_read);
            }
            else if (evList[i].filter == EVFILT_WRITE)
            {
                printf("Ok to write more!\n");

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
    }

    return 0;
}