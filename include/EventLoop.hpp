#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"
#include "netpoll/NetPoll.hpp"
#include "Interface.hpp"
#include "NetConn.hpp"
#include "NetListener.hpp"
#include "NetSocket.hpp"

namespace cqnet {

class EventLoop
    : public IEventLoop
    , public base::NonCopyable
    , public std::enable_shared_from_this<EventLoop>
{
private:
    int index_;
    std::shared_ptr<INetPoll> poller_;
    std::atomic<int32_t> conn_count_;
    std::unordered_map<int, NetConn*> connections_;
    cqnet::TcpListener::Ptr tcp_listener_{nullptr};
    // some interface for user-defined logic
    std::shared_ptr<ICodec> codec_;
    std::shared_ptr<IEventHandler> event_handler_;

protected:
    EventLoop(int index, std::shared_ptr<ICodec> codec, std::shared_ptr<IEventHandler> event_handler)
        : index_(index)
        , codec_(std::move(codec))
        , conn_count_(0)
        , event_handler_(std::move(event_handler))
        , poller_(netpoll::Poller::Create()){};

    ~EventLoop() = default;

public:
    using Ptr = std::shared_ptr<EventLoop>;

    Ptr static Create(int index, std::shared_ptr<ICodec> codec, std::shared_ptr<IEventHandler> event_handler)
    {
        class make_shared_enabler : public EventLoop
        {
        public:
            make_shared_enabler(int index, std::shared_ptr<ICodec> codec, std::shared_ptr<IEventHandler> event_handler)
                : EventLoop(index, std::move(codec), std::move(event_handler))
            {
            }
        };

        return std::make_shared<make_shared_enabler>(index, std::move(codec), std::move(event_handler));
    }

    void Run() override
    {
        auto func = std::bind(
            &EventLoop::HandleEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        poller_->Polling(std::move(func));
    }

    // add a new tcp connection to event loop
    void AddNewConn(int new_conn_fd) override
    {
        auto conn = new NetConn(new_conn_fd, this->codec_, this->poller_);
        conn->SetNonblock();

        if (poller_->AddRead(new_conn_fd, (void*)conn))
        {
            // add the connection to the connection map
            connections_.insert(std::pair<int, NetConn*>(new_conn_fd, conn));
            PlusConnCount();
        }

        auto ret = event_handler_->OnOpened(conn);
        conn->AsyncWrite(std::get<0>(ret));
    }

    bool AddTcpListener(std::shared_ptr<ILoadBalance> lb, bool is_IPV6, const char* ip, int port, int back_num) override
    {
        tcp_listener_ = cqnet::TcpListener::Create(std::move(lb), is_IPV6, ip, port, back_num);
        return poller_->AddRead(tcp_listener_->GetFD(), (void*)tcp_listener_.get());
    }

private:
    // HandleEvent defines the handler to net event
    bool HandleEvent(int fd, int16_t filter, Socket* socket)
    {
        if (filter == EVFILT_WRITE)
        {
            std::cout << "WRITE 事件, fd = " << fd << std::endl;
            return socket->Write();
        }
        else if (filter == EVFILT_READ)
        {
            std::cout << "READ 事件, fd = " << fd << std::endl;
            // read socket
            socket->Read();
            auto iter = connections_.find(fd);
            if (iter != connections_.end())
            {
                auto conn = iter->second;
                auto r = event_handler_->React(codec_->Decode(conn));
                event_handler_->PreWrite();
                iter->second->AsyncWrite(std::get<0>(r));
            }
        }
        else
        {
            auto iter = connections_.find(fd);
            if (iter == connections_.end())
            {
                return false;
            }
            auto conn_ptr = iter->second;
            CloseConn(conn_ptr);
        }

        return true;
    }

    bool WakeConn(NetConn* conn)
    {
        std::cout << "WakeConn" << std::endl;
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
        case Action::None:
            std::cout << "None" << std::endl;
            break;
        case Action::Close:
            std::cout << "Close" << std::endl;
            break;
        case Action::ShutDown:
            std::cout << "Shutdown" << std::endl;
            break;
        }

        delete conn;
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
};

} // namespace cqnet
