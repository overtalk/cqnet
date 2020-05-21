#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"
#include "netpoll/NetPoll.hpp"
#include "Interface.hpp"
#include "NetConn.hpp"
#include "CQNetServer.hpp"

namespace cqnet {

class EventLoop
    : public base::NonCopyable
    , public std::enable_shared_from_this<EventLoop>
{
private:
    int index_; // event loop index
    CQNetServer::Ptr svr_;
    NetConn::EncodeFunc encode_func_;
    NetConn::DecodeFunc decode_func_;
    std::unordered_map<int, NetConn::Ptr> connections_;
    netpoll::KQueue::Ptr kqueue_;     // epoll or kqueue
    std::atomic<int32_t> conn_count_; // number of active connections in event-loop
    // some interface
    std::shared_ptr<ICodec> codec_;
    std::shared_ptr<IEventHandler> event_handler_;

protected:
    EventLoop(int index, std::shared_ptr<ICodec> codec, std::shared_ptr<IEventHandler> event_handler,
        std::shared_ptr<CQNetServer> server)
        : index_(index)
        , codec_(codec)
        , event_handler_(event_handler)
        , svr_(server)
        , kqueue_(netpoll::KQueue::Create())
    {
        auto encode_func = [codec](NetConn::Ptr conn, char* buf) { return codec->Encode(conn, buf); };
        auto decode_func = [codec](NetConn::Ptr conn) { return codec->Decode(conn); };

        encode_func_ = std::forward<NetConn::EncodeFunc>(encode_func);
        decode_func_ = std::forward<NetConn::DecodeFunc>(decode_func);
    };

    ~EventLoop() {}

public:
    using Ptr = std::shared_ptr<EventLoop>;

    Ptr static Create(int index, std::shared_ptr<ICodec> codec, std::shared_ptr<IEventHandler> event_handler,
        std::shared_ptr<CQNetServer> server)
    {
        class make_shared_enabler : public EventLoop
        {
        public:
            make_shared_enabler(int index, std::shared_ptr<ICodec> codec, std::shared_ptr<IEventHandler> event_handler,
                std::shared_ptr<CQNetServer> server)
                : EventLoop(index, std::move(codec), std::move(event_handler), std::move(server))
            {
            }
        };

        return std::make_shared<make_shared_enabler>(
            index, std::move(codec), std::move(event_handler), std::move(server));
    }

    void Run()
    {
        auto function = std::bind(&EventLoop::HandleEvent, this, std::placeholders::_1, std::placeholders::_2);
        kqueue_->Polling(std::move(function));
    }

    // TODO: del this later
    netpoll::KQueue::Ptr GetKQueue()
    {
        return kqueue_;
    }

private:
    // HandleEvent defines the handler to net event
    bool HandleEvent(int fd, int16_t filter)
    {
        auto iter = connections_.find(fd);
        if (iter != connections_.end())
        {
            std::shared_ptr<NetConn> conn = iter->second;

            if (!conn->IsOutBufferEmpty())
            {
                if (filter == EVFILT_WRITE)
                {
                    return WriteConn(std::move(conn));
                }
            }
            else
            {
                if (filter == EVFILT_READ)
                {
                    return ReadConn(std::move(conn));
                }
            }
            return true;
        }

        return AcceptConn(fd);
    }

    // accept a new tcp connection
    bool AcceptConn(int fd)
    {
        // 判断是否为 listen socket 发送了事件
        if (svr_->GetFD() == fd)
        {
            int new_conn_fd = svr_->Read(nullptr, 0);
            if (new_conn_fd == CQNET_INVALID_SOCKET)
            {
                //TODO: error handle
            }

            auto conn = NetConn::Create(new_conn_fd, this->kqueue_, encode_func_, decode_func_);
            conn->SetNonblock();

            if (kqueue_->AddRead(new_conn_fd))
            {
                connections_.insert(std::pair<int, NetConn::Ptr>(new_conn_fd, conn));
                PlusConnCount();
                return true;
            }

            if (!OpenConn(conn))
            {
                // TODO: error handle
            }
        }

        return false;
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
