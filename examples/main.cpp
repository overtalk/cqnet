//#include <iostream>
//#include <unistd.h>
//#include "netpoll/NetPoll.hpp"
#include "components/Codec.hpp"
#include "components/RoundRobin.hpp"
#include "components/EventHandler.hpp"
#include "CQNetServer.hpp"

int main(int argc, const char* argv[])
{
    auto cqnet_server = cqnet::CQNetServer::Create(2, std::make_shared<cqnet::Codec>(),
        std::make_shared<cqnet::RoundRobinLoadBalance>(), std::make_shared<cqnet::EventHandler>());
    cqnet_server->AddTcp(false, "127.0.0.1", 9999);
    cqnet_server->Run();

    while (true)
    {
        usleep(1000);
    }

    return 0;
}