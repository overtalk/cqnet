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
    netpoll::KQueue::Ptr kqueue_;     // epoll or kqueue
    std::atomic<int32_t> conn_count_; // number of active connections in event-loop
    std::unordered_map<int, NetConn::Ptr> connections_;
    // some interface for user-defined logic
    std::shared_ptr<ICodec> codec_;
    std::shared_ptr<IEventHandler> event_handler_;

protected:
    EventLoop(int index, std::shared_ptr<ICodec> codec, std::shared_ptr<IEventHandler> event_handler)
        : index_(index)
        , codec_(codec)
        , event_handler_(event_handler)
        , kqueue_(netpoll::KQueue::Create()){};

    ~EventLoop() {}

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

    void Run()
    {
        auto func = std::bind(
            &EventLoop::HandleEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        kqueue_->Polling(std::move(func));
    }

    // TODO: del this later
    netpoll::KQueue::Ptr GetKQueue()
    {
        return kqueue_;
    }

    // add a new tcp connection to event loop
    void AddNewConn(int new_conn_fd) override
    {
        auto conn = NetConn::Create(new_conn_fd, this->kqueue_);
        conn->SetNonblock();

        if (kqueue_->AddRead(new_conn_fd, (void*)conn.get()))
        {
            // add the connection to the connection map
            connections_.insert(std::pair<int, NetConn::Ptr>(new_conn_fd, conn));
            PlusConnCount();
        }
    }

    void AddTcpListener(TcpListener::Ptr l)
    {
        kqueue_->AddRead(l->GetFD(), (void*)l.get());
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
                auto conn_ptr = iter->second;
                codec_->Decode(conn_ptr);
                //                do
                //                {
                //                    // TODO : use codec to get a package
                //                    // TODO: use event_handler
                //                    break;
                //                } while (true);
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

    bool OpenConn(NetConn::Ptr conn)
    {
        // TODO: set conn address
        auto tup = event_handler_->OnOpened(conn);

        // TODO: handle all those things
        auto data = std::get<0>(tup);
        auto action = std::get<1>(tup);

        return true;
    }

    bool ReadConn(NetConn::Ptr conn)
    {
        std::cout << "ReadConn" << std::endl;
        return true;
    }

    bool WriteConn(NetConn::Ptr conn)
    {
        std::cout << "WriteConn" << std::endl;
        return true;
    }

    bool WakeConn(NetConn::Ptr conn)
    {
        std::cout << "WakeConn" << std::endl;
        return true;
    }

    void CloseAllConns()
    {
        for (auto& conn : connections_)
        {
            CloseConn(std::move(conn.second));
        }
    }

    bool CloseConn(NetConn::Ptr conn)
    {
        if (!conn->IsOutBufferEmpty())
        {
            // TODO: write to client
        }

        int fd = conn->GetFD();
        kqueue_->Delete(fd);
        conn->Close();
        // delete from the map
        connections_.erase(fd);
        MinusConnCount();

        switch (event_handler_->OnClosed(conn))
        {
        case Action::None:
            break;
        case Action::Close:
            break;
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
};

} // namespace cqnet
