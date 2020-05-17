#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"
#include "base/Buffer.hpp"
#include "server/NetAddr.hpp"
#include "server/Codec.hpp"
#include "server/EventLoop.hpp"
#include "server/NetSocket.hpp"

namespace cqnet {

// conn should hold the fd of socket
class NetConn : public ConnSocket
{
private:
    bool opened_;              // connection opened event fired
    Codec::Ptr codec_;         // codec for TCP
    EventLoop::Ptr loop_;      // connected event-loop
    NetAddr::Ptr local_addr_;  // local addr
    NetAddr::Ptr remote_addr_; // remote addr
    // 这个buffer在eventloop中读取数据的时候就直接读到这个里面
    base::CharBuffer inbound_buffer_;  // store data to read
    base::CharBuffer outbound_buffer_; // store data to send
    // 读socket数据时候的缓冲队列
    char recv_buffer_[1024];
    int recv_size;

public:
    using Ptr = std::shared_ptr<NetConn>;

    // 在 event loop 中， 每次 accept 到一个 fd，就调用这个方法创建一个 connection
    // 然后将 read 事件注册到 epoll 中， 每次有read 事件，就调用这个 connection 的
    Ptr static Create(int fd, EventLoop::Ptr loop, Codec::Ptr codec)
    {
        class make_shared_enabler : public NetConn
        {
        public:
            make_shared_enabler(int fd, EventLoop::Ptr loop, Codec::Ptr codec)
                : NetConn(fd, std::move(loop), std::move(codec))
            {
            }
        };

        return std::make_shared<make_shared_enabler>(fd, std::move(loop), std::move(codec));
    }

protected:
    NetConn(int fd, EventLoop::Ptr loop, Codec::Ptr codec_)
        : opened_(false)
        , ConnSocket(fd, true)
        , codec_(std::move(codec_))
        , loop_(std::move(loop))
    {
    }

    ~NetConn();

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
            loop_->GetPoller()->AddWrite(GetFD());
        }
        return true;
    }

    bool Wakeup()
    {
        EventLoop::Ptr event_loop = this->loop_;

        auto cb = [event_loop]() {
            // TODO: modify the function
            auto c = [] { return false; };
            event_loop->GetPoller()->Trigger(c);
            return false;
        };

        return loop_->GetPoller()->Trigger(cb);
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
