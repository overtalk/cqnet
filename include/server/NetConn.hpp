#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"
#include "base/RingBuffer.hpp"
#include "server/Codec.hpp"

namespace cqnet {

class EventLoop;
class NetAddr;

// TODO: add ctx
// conn should hold the fd
class NetConn : public base::Noncopyable
{
private:
    int fd_;                     // file descriptor
    const EventLoop* loop_;      // connected event-loop
    Codec* codec_;               // codec for TCP
    bool opened_;                // connection opened event fired
    const NetAddr* local_addr_;  // local addr
    const NetAddr* remote_addr_; // remote addr
    base::RingBuffer<char> inbound_buffer_;
    base::RingBuffer<char> outbound_buffer_;

public:
    NetConn();
    ~NetConn();

    char* Read()
    {
        if (inbound_buffer_.IsEmpty())
        {
            // TODO:
        }
    }

private:
    void open(char* buf) {}

    char* read()
    {
        return codec_->Decode(this);
    }

    int write(char* buf, size_t len)
    {
        if (!outbound_buffer_.IsEmpty())
        {
            outbound_buffer_.Write(buf, len);
        }

        // TODO: send will return the bytes sended ????
        int transnum = send(fd_, buf, len, 0);
        if (transnum < 0)
        {
            transnum = 0;
        }

        /*  send error if transnum < 0  */
        // TODO: error handler, close this connection
        return transnum;
    }
};

} // namespace cqnet
