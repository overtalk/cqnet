#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"
#include "netpoll/NetPoll.hpp"
#include "server/NetConn.hpp"
#include "server/EventHandler.hpp"
#include "server/CQNetServer.hpp"

namespace cqnet {

// 他的功能就是监听io事件，然后调用对应的处理函数
// 并且需要保存链接
// RUN 方法启动，会一直监听事件，然后去处理
class EventLoop : public base::Noncopyable
{
public:
    using Ptr = std::shared_ptr<EventLoop>;

private:
    int index_;        // event loop index
    Codec::Ptr codec_; // codec for TCP
    CQNetServer* svr_;
    netpoll::Poller* poller_;         // epoll or kqueue
    std::atomic<int32_t> conn_count_; // number of active connections in event-loop
    std::unordered_map<int, NetConn*> connections_;
    EventHandler* event_handler_;

public:
    Ptr static Create()
    {
        class make_shared_enabler : public EventLoop
        {
        };
        return std::make_shared<make_shared_enabler>();
    }

    netpoll::Poller* GetPoller()
    {

        return poller_;
    }

private:
    void Run()
    {
        // TODO: add a goroutine for ticker

        // poller_->Polling(HandleEvent);
    }

    bool HandleEvent(int fd, int16_t filter)
    {
        auto iter = connections_.find(fd);
        if (iter != connections_.end())
        {
            NetConn* conn = iter->second;
            switch (conn->IsOutBufferEmpty())
            {
            case false:
                if (filter == EVFILT_WRITE)
                {
                    // TODO: write
                }
                break;

            default:
                if (filter == EVFILT_READ)
                {
                    // TODO: read
                }
                break;
            }

            return true;
        }

        return Accept(fd);
    }

    // accept a new tcp connection
    bool Accept(int fd)
    {
        if (svr_->GetListenerFD() == fd)
        {
            int new_conn_fd = ::accept(fd, nullptr, nullptr);

            // TODO: use smart ptr
            NetConn* conn = new NetConn(new_conn_fd, this);
            // TODO: set nonblock

            if (poller_->AddRead(new_conn_fd))
            {
                connections_.insert(std::pair<int, NetConn*>(new_conn_fd, conn));
                PlusConnCount();
                return true;
            }
        }

        return false;
    }

    bool OpenConn(NetConn* conn)
    {
        // TODO: set conn address
        auto tup = event_handler_->OnOpened(conn);

        // TODO: handle all those things
        auto data = std::get<0>(tup);
        auto action = std::get<1>(tup);
    }

    bool ReadConn(NetConn* conn)
    {
        return true;
    }

    bool WriteConn(NetConn* conn)
    {
        return true;
    }

    bool WakeConn(NetConn* conn)
    {
        return true;
    }

    void CloseAllConns()
    {
        for (auto& conn : connections_)
        {
            CloseConn(conn.second);
        }
    }

    bool CloseConn(NetConn* conn)
    {
        if (!conn->IsOutBufferEmpty())
        {
            // TODO: write to client
        }

        int fd = conn->GetFD();
        poller_->Delete(fd);
        conn->Close();
        // delete from the map
        connections_.erase(fd);
        MinusConnCount();

        switch (event_handler_->OnClosed(conn))
        {
        case Action::ShutDown:
            break;
        }

        // TODO: release connection
        return true;
    }

    // some easy function
    void PlusConnCount()
    {
        conn_count_++;
    }

    void MinusConnCount()
    {
        conn_count_--;
    }

    int32_t LoadConnCount()
    {
        return conn_count_.load();
    }

protected:
    EventLoop() = default;
};

} // namespace cqnet
