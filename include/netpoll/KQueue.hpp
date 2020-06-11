#pragma once

#include "base/Platform.hpp"
#include "base/AsyncJobQueue.hpp"
#include "base/NonCopyable.hpp"
#include "NetSocket.hpp"
#include "Interface.hpp"

namespace cqnet {
namespace netpoll {

class Poller
    : public base::NonCopyable
    , public INetPoll
{
public:
    // EVFilterSock represents exceptional events that are not read/write, like socket being closed,
    // reading/writing from/to a closed socket, etc.
    static const int event_filter_sock_;
    // for wake up
    static const int user_event_ident_;
    // init_event_size_ represents the initial length of poller event-list.
    static const int init_event_list_size_;
    // event_list_resize_increment_ represents the increment of event list vector
    static const int event_list_resize_increment_;

private:
    int kq_fd_;                           // kqueue fd
    base::AsyncJobQueue async_job_queue_; // async job queue

protected:
    Poller()
    {
        kq_fd_ = kqueue();
        if (kq_fd_ == -1)
        {
            std::cout << "failed to create kq, kq id = " << kq_fd_ << std::endl;
            // TODO: error handler, failed to new kqueue
        }

        if (!add_user_event())
        {
            // TODO: failed to add user-defined event
        }
    }

    ~Poller() {}

public:
    using Callback = std::function<bool(int, int16_t, cqnet::Socket*)>;
    using Ptr = std::shared_ptr<Poller>;

    Ptr static Create()
    {
        class make_shared_enabler : public Poller
        {
        public:
            make_shared_enabler()
                : Poller()
            {
            }
        };

        return std::make_shared<make_shared_enabler>();
    }

    // Close closes the poller.
    bool Close()
    {
        return close(kq_fd_) >= 0;
    }

    // Trigger wakes up the poller blocked in waiting for network-events and runs jobs in asyncJobQueue.
    bool Trigger(const base::AsyncJobQueue::AsyncJob& job)
    {
        if (async_job_queue_.Push(job) == 1)
        {
            return wake_up();
        }
        return true;
    }

    // Polling blocks the current thread, waiting for network-events.
    bool Polling(Callback cb)
    {
        bool wake_up = false;
        std::vector<struct kevent> kq_events_(init_event_list_size_);

        int milliseconds = 0;
        while (true)
        {
            struct timespec timeout = {milliseconds / 1000, (milliseconds % 1000) * 1000 * 1000};
            int active_kq_events_num_ = kevent(kq_fd_, nullptr, 0, kq_events_.data(), kq_events_.size(), &timeout);
            for (int i = 0; i < active_kq_events_num_; ++i)
            {
                const struct kevent& event = kq_events_[i];
                int fd = event.ident;
                int16_t ev_filter = kq_events_[i].filter;
                uint16_t ev_flags = kq_events_[i].flags;
                auto socket = (cqnet::Socket*)kq_events_[i].udata;

                if (fd != user_event_ident_)
                {
                    if (((ev_flags & EV_EOF) != 0) || ((ev_flags & EV_ERROR) != 0))
                    {
                        std::cout << "EV_EOF / EV_ERROR" << std::endl;
                        ev_filter = event_filter_sock_;
                    }

                    if (!cb(fd, ev_filter, socket))
                    {
                        return false;
                    }
                }
                else
                {
                    wake_up = true;
                }

                if (wake_up)
                {
                    wake_up = false;
                    if (!async_job_queue_.FroEach())
                    {
                        return false;
                    }
                }

                if (active_kq_events_num_ == kq_events_.size())
                {
                    kq_events_.resize(active_kq_events_num_ + event_list_resize_increment_);
                }
            }
        }
        return true;
    }

    // add kevent
    bool AddRead(const int& fd, void* meta)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, meta);
        return kevent(kq_fd_, &evSet, 1, nullptr, 0, nullptr) == 0;
    }

    bool AddWrite(const int& fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD, 0, 0, nullptr);
        return kevent(kq_fd_, &evSet, 1, nullptr, 0, nullptr) == 0;
    }

    bool AddReadWrite(const int& fd, void* meta)
    {
        struct kevent evSet[2];
        EV_SET(&evSet[0], fd, EVFILT_READ, EV_ADD, 0, 0, meta);
        EV_SET(&evSet[1], fd, EVFILT_WRITE, EV_ADD, 0, 0, meta);
        return kevent(kq_fd_, evSet, 2, nullptr, 0, nullptr) == 0;
    }

    // delete kevent
    bool DelRead(const int& fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        return kevent(kq_fd_, &evSet, 1, nullptr, 0, nullptr) == 0;
    }

    bool DelWrite(const int& fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
        return kevent(kq_fd_, &evSet, 1, nullptr, 0, nullptr) == 0;
    }

    bool Delete(const int& fd)
    {
        // Calling close() on a file descriptor will remove any kevents that reference the descriptor.
        // so if you want to remove all the evenet in kqueue, just close the fd which should called by user
        // https://www.freebsd.org/cgi/man.cgi?query=kqueue&sektion=2
        return true;

        // struct kevent evSet[2];
        // EV_SET(&evSet[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        // EV_SET(&evSet[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
        // return KQEventChange(evSet, 2);
    }

private:
    bool add_user_event()
    {
        // add user event for make up
        struct kevent ev;
        EV_SET(&ev, user_event_ident_, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, nullptr);

        struct timespec timeout = {0, 0};
        return kevent(kq_fd_, &ev, 1, NULL, 0, &timeout) == 0;
    }

    bool wake_up()
    {
        struct kevent ev;
        EV_SET(&ev, user_event_ident_, EVFILT_USER, 0, NOTE_TRIGGER, 0, nullptr);

        struct timespec timeout = {0, 0};
        return kevent(kq_fd_, &ev, 1, NULL, 0, &timeout) == 0;
    }
};

const int Poller::user_event_ident_ = 0;
const int Poller::init_event_list_size_ = 64;
const int Poller::event_list_resize_increment_ = 128;
const int Poller::event_filter_sock_ = -0xd;

} // namespace netpoll
} // namespace cqnet
