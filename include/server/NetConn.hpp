#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"
#include "base/Buffer.hpp"
#include "server/Codec.hpp"
#include "server/EventLoop.hpp"

namespace cqnet {

class NetAddr;

// conn should hold the fd of socket
class NetConn : public base::Noncopyable
{
private:
    int fd_;                     // file descriptor
    bool opened_;                // connection opened event fired
    Codec* codec_;               // codec for TCP
    EventLoop* loop_;            // connected event-loop
    const NetAddr* local_addr_;  // local addr
    const NetAddr* remote_addr_; // remote addr
    // 这个buffer在eventloop中读取数据的时候就直接读到这个里面
    base::CharBuffer inbound_buffer_;  // store data to read
    base::CharBuffer outbound_buffer_; // store data to send

public:
    NetConn();
    ~NetConn();

    // this is used for Codec
    std::tuple<char*, size_t> Read()
    {
        return inbound_buffer_.get_data_with_readable_size();
    }

    bool ShiftN(size_t size)
    {
        return inbound_buffer_.shift_n(size);
    }

    size_t BufferSize()
    {
        return inbound_buffer_.get_readable_count();
    }

    void ResetBuffer()
    {
        inbound_buffer_.reset();
    }

    // TODO: how to use Encode func, this is a problem
    bool AsyncWrite(const char* data, size_t size) {}

    bool Wakeup()
    {
        EventLoop* event_loop = this->loop_;

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

    const NetAddr* LocalAddr()
    {
        return local_addr_;
    }

    const NetAddr* RemoteAddr()
    {
        return remote_addr_;
    }

    void Context() {}
    void SetContext() {}

private:
    void open(char* buf) {}

    char* read()
    {
        return codec_->Decode(this);
    }

    void write(const char* data, size_t size)
    {
        // 还有数据没有发送出去，排队发送
        // 当 outbound_buffer_ 不为空的时候，一定会增加这个connection fd 的可写事件的。所以可以放心的return
        if (!outbound_buffer_.is_empty())
        {
            outbound_buffer_.write(data, size);
            return;
        }

        /*  send error if transnum < 0  */
        int send_size = ::send(fd_, data, size, 0) < 0;
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
            loop_->GetPoller()->AddWrite(fd_);
        }
    }
};

} // namespace cqnet
