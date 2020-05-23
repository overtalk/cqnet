#include <sys/un.h>
#include <iostream>
#include "base/SocketLib.hpp"
#include "netpoll/NetPoll.hpp"
#include "components/Codec.hpp"
#include "components/RoundRobin.hpp"
#include "components/EventHandler.hpp"
#include "EventLoop.hpp"
#include "CQNetServer.hpp"

int main(int argc, const char* argv[])
{
    auto el = cqnet::EventLoop::Create(1, std::make_shared<cqnet::Codec>(), std::make_shared<cqnet::EventHandler>());
    auto lb = std::make_shared<cqnet::RoundRobinLoadBalance>();
    lb->Register(el);

    auto l = cqnet::TcpListener::Create(lb, false, "127.0.0.1", 9999);
    std::cout << " yes " << std::endl;
    el->AddTcpListener(l);

    el->Run();

    return 0;
}