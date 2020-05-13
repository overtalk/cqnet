#pragma once

#include "base/Platform.hpp"
#include "base/AsyncJobQueue.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {

class Poller : public base::Noncopyable
{
public:
    // EVFilterSock represents exceptional events that are not read/write, like socket being closed,
    // reading/writing from/to a closed socket, etc.
    static int EVFilterSock;
    // for wake up
    static int user_event_ident_;
    // init_event_size_ represents the initial length of poller event-list.
    static int init_event_size_;
    // event_list_resize_increment_ represents the increment of event list vector
    static int event_list_resize_increment_;

private:
    int kq_fd_;                           // kqueue fd
    base::AsyncJobQueue async_job_queue_; // async job queue

private:
    class PollerDeleter
    {
    public:
        void operator()(Poller* ptr) const
        {
            delete ptr;
        }
    };

public:
    using Ptr = std::unique_ptr<Poller, PollerDeleter>;
    using Callback = std::function<bool(int, int16_t)>;

public:
    static Poller::Ptr OpenPoller()
    {
        class make_unique_enabler : public Poller
        {
        public:
            make_unique_enabler() {}
        };

        return Ptr(new make_unique_enabler());
    }

    // Polling blocks the current thread, waiting for network-events.
    bool Polling(Callback& cb)
    {
        bool wake_up = false;
        std::vector<struct kevent> kq_events_(init_event_size_); // for receive entry
        while (true)
        {
            // struct timespec timeout = {milliseconds / 1000, (milliseconds % 1000) * 1000 * 1000};
            int active_kq_events_num_ = kevent(kq_fd_, nullptr, 0, kq_events_.data(), kq_events_.size(), nullptr);
            for (int i = 0; i < active_kq_events_num_; ++i)
            {
                uint16_t fd = kq_events_[i].ident;
                if (fd != user_event_ident_)
                {
                    int16_t ev_filter = kq_events_[i].filter;
                    if ((kq_events_[i].flags & EV_EOF != 0) || (kq_events_[i].flags & EV_ERROR != 0))
                    {
                        ev_filter = EVFilterSock;
                    }

                    if (!cb(fd, ev_filter))
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
            return Wakeup();
        }
        return true;
    }

    // add kevent
    bool AddRead(const int& fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        return KQueueEventChange(&evSet);
    }

    bool AddWrite(const int& fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
        return KQueueEventChange(&evSet);
    }

    bool AddReadWrite(const int& fd)
    {
        struct kevent evSet[2];
        EV_SET(&evSet[0], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        EV_SET(&evSet[1], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
        return KQueueEventChange(evSet, 2);
    }

    // delete kevent
    bool DelRead(const int& fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        return KQueueEventChange(&evSet);
    }

    bool DelWrite(const int& fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        return KQueueEventChange(&evSet);
    }

    bool Delete(const int& fd)
    {
        struct kevent evSet[2];
        EV_SET(&evSet[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        EV_SET(&evSet[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        return KQueueEventChange(evSet, 2);
    }

private:
    bool Wakeup()
    {
        struct kevent ev;
        EV_SET(&ev, user_event_ident_, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
        return KQueueEventChange(&ev);
    }

    bool KQueueEventChange(const struct kevent* changelist, int nchanges = 1)
    {
        if (kevent(kq_fd_, changelist, nchanges, nullptr, 0, nullptr) < 0)
        {
            return false;
        }

        return true;
    }

protected:
    Poller()
    {
        // async_job_queue_ = base::AsyncJobQueue();
        kq_fd_ = kqueue();
        struct kevent evSet;
        EV_SET(&evSet, user_event_ident_, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);
        if (!KQueueEventChange(&evSet))
        {
            // TODO: error handle
        }

        // TODO: del
        std::cout << "kqueue fd = " << kq_fd_ << std::endl;
    }
};

int Poller::user_event_ident_ = 0;
int Poller::init_event_size_ = 64;
int Poller::event_list_resize_increment_ = 128;
int Poller::EVFilterSock = -0xd;

} // namespace cqnet
