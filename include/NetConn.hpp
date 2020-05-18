#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"
#include "base/Buffer.hpp"
#include "netpoll/NetPoll.hpp"
#include "NetAddr.hpp"
#include "NetSocket.hpp"

namespace cqnet {

class NetConn : public ConnSocket
{
public:
    using Ptr = std::shared_ptr<NetConn>;
    using EncodeFunc = std::function<char*(std::shared_ptr<NetConn>, char*)>;
    using DecodeFunc = std::function<char*(std::shared_ptr<NetConn>)>;

private:
    bool opened_;              // connection opened event fired
    int recv_size;
    char recv_buffer_[1024];
    EncodeFunc encode_func_;
    DecodeFunc decode_func_;
    NetAddr::Ptr local_addr_;  // local addr
    NetAddr::Ptr remote_addr_; // remote addr
    netpoll::KQueue::Ptr kqueue_;  // connected event-loop
    base::CharBuffer inbound_buffer_;  // store data to read
    base::CharBuffer outbound_buffer_; // store data to send

protected:
    NetConn(int fd, netpoll::KQueue::Ptr kqueue, EncodeFunc encode_func, DecodeFunc decode_func)
        : ConnSocket(fd, true)
        , opened_(false)
        , recv_size(0)
        , encode_func_(std::move(encode_func))
        , decode_func_(std::move(decode_func))
        , kqueue_(std::move(kqueue))
        , inbound_buffer_( base::CharBuffer())
        , outbound_buffer_( base::CharBuffer(0))
    {
        //TODO: net address
    }

    ~NetConn(){}

public:
    // 在 event loop 中， 每次 accept 到一个 fd，就调用这个方法创建一个 connection
    // 然后将 read 事件注册到 epoll 中， 每次有read 事件，就调用这个 connection 的
    Ptr static Create(int fd, netpoll::KQueue::Ptr kqueue, EncodeFunc encode_func, DecodeFunc decode_func)
    {
        class make_shared_enabler : public NetConn
        {
        public:
            make_shared_enabler(int fd, netpoll::KQueue::Ptr kqueue, EncodeFunc encode_func, DecodeFunc decode_func)
                :NetConn(fd, std::move(kqueue), std::move(encode_func), std::move(decode_func))
            {}
        };

        return std::make_shared<make_shared_enabler>(fd, std::move(kqueue), std::move(encode_func), std::move(decode_func));
    }

public:
    // this is used for Codec
    // 从 buffer 中读取数据出来撒
    std::tuple<char*, size_t> ReadBuffer()
    {
        return inbound_buffer_.get_read_ptr_with_readable_count();
    }

    // 从socket中读取数据
    // 只有当 event loop 从 epoll 中得到当前的 connection 有可读事件的时候，然后在 epoll 中调用这个方法
    // 将 socket 中的数据读取出来
    void RecvFromSocket()
    {
        do
        {
            recv_size = base::SocketRecv(GetFD(), recv_buffer_, 1024);
            inbound_buffer_.write(recv_buffer_, recv_size);
        } while (recv_size < 1024);
    }

    bool ShiftN(size_t size)
    {
        return inbound_buffer_.shift_n(size);
    }

    void ResetBuffer()
    {
        inbound_buffer_.reset();
    }

    bool AsyncWrite(char* data, size_t size)
    {
        // TODO: call encode function , this is a problem

        // 还有数据没有发送出去，排队发送
        // 当 outbound_buffer_ 不为空的时候，一定会增加这个connection fd 的可写事件的。所以可以放心的return
        if (!outbound_buffer_.is_empty())
        {
            outbound_buffer_.write(data, size);
            return true;
        }

        /*  send error if transnum < 0  */
        int send_size = SendToSocket(data, size);
        if (send_size < 0)
        {
            // TODO: close connection
            // if Eagain，write to buffer
            // or return an error
        }

        // if send_size < size
        if (send_size < size)
        {
            outbound_buffer_.write(data + send_size, size - send_size);
            kqueue_->AddWrite(GetFD());
        }
        return true;
    }

    bool Wakeup()
    {
        auto kqueue = this->kqueue_;

        auto cb = [kqueue]() {
            // TODO: modify the function
            auto c = [] { return false; };
            kqueue->Trigger(c);
            return false;
        };

        return kqueue_->Trigger(cb);
    }

    bool Close()
    {
        // TODO: close
        return true;
    }

    // 如果 out buffer 为空，则表示 epoll 中没有当前 fd 的读事件
    // 如果不为空的话，就有读事件
    bool IsOutBufferEmpty()
    {
        return outbound_buffer_.is_empty();
    }

    NetAddr::Ptr LocalAddr()
    {
        return local_addr_;
    }

    NetAddr::Ptr RemoteAddr()
    {
        return remote_addr_;
    }

    void Context() {}
    void SetContext() {}

private:
    void open(char* buf) {}
};

} // namespace cqnet