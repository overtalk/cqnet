#include <sys/un.h>
#include <iostream>
#include "base/SocketLib.hpp"
#include "netpoll/NetPoll.hpp"
#include "components/Codec.hpp"
#include "components/EventHandler.hpp"
#include "EventLoop.hpp"
#include "EventLoop.hpp"
#include "CQNetServer.hpp"

int main(int argc, const char* argv[])
{
    // create listen fd
    int localFd = cqnet::base::SocketTcpListen(false, "127.0.0.1", 9999, 512);
    std::cout << localFd << std::endl;

//    auto kq_ptr = cqnet::netpoll::KQueue::Create();
//    kq_ptr->AddRead(localFd);
//
//    kq_ptr->Polling([](int fd, int16_t f){
//      std::cout << fd << " - " << f << std::endl;
//      return true;
//    });



    auto el = cqnet::EventLoop::Create(1, std::make_shared<cqnet::Codec>(), std::make_shared<cqnet::EventHandler>());

    auto kqueue = el->GetKQueue();
    kqueue->AddRead(localFd);

    el->Run();

    return 0;
}