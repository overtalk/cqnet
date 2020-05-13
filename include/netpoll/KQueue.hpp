#pragma once

#include "base/Platform.hpp"
#include "base/AsyncJobQueue.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {

auto EVFilterSock = -0xd;

class Poller : public base::Noncopyable
{
private:
    int kq_fd_;                           // kqueue fd
    int user_event_ident_;                // for wake up
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
        std::vector<struct kevent> event_entries; // for receive entry
        while (true)
        {
            // struct timespec timeout = {milliseconds / 1000, (milliseconds % 1000) * 1000 * 1000};
            int num_complete = kevent(kq_fd_, NULL, 0, event_entries.data(), event_entries.size(), nullptr);
            for (int i = 0; i < num_complete; ++i)
            {
                uint16_t fd = event_entries[i].ident;
                if (fd != user_event_ident_)
                {
                    int16_t ev_filter = event_entries[i].filter;
                    if ((event_entries[i].flags & EV_EOF != 0) || (event_entries[i].flags & EV_ERROR != 0))
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

                if (num_complete == event_entries.size())
                {
                    event_entries.resize(num_complete + 128);
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
    bool Trigger(base::AsyncJobQueue::AsyncJob job)
    {
        if (async_job_queue_.Push(job) == 1)
        {
            Wakeup();
        }
        return true;
    }

    // add kevent
    bool AddRead(int fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        return KQueueEventChange(&evSet);
    }

    bool AddWrite(int fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
        return KQueueEventChange(&evSet);
    }

    bool AddReadWrite(int fd)
    {
        struct kevent evSet[2];
        EV_SET(&evSet[0], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        EV_SET(&evSet[1], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
        return KQueueEventChange(evSet, 2);
    }

    // delete kevent
    bool DelRead(int fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        return KQueueEventChange(&evSet);
    }

    bool DelWrite(int fd)
    {
        struct kevent evSet;
        EV_SET(&evSet, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        return KQueueEventChange(&evSet);
    }

    bool DelReadWrite(int fd)
    {
        struct kevent evSet[2];
        EV_SET(&evSet[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        EV_SET(&evSet[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        return KQueueEventChange(evSet, 2);
    }

private:
    void Wakeup()
    {
        struct kevent ev;
        EV_SET(&ev, user_event_ident_, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
        KQueueEventChange(&ev);
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
        : user_event_ident_(0)
    {
        // async_job_queue_ = base::AsyncJobQueue();
        kq_fd_ = kqueue();
        struct kevent evSet;
        EV_SET(&evSet, user_event_ident_, EV_ADD | EV_CLEAR, EVFILT_USER, 0, 0, NULL);
        if (!KQueueEventChange(&evSet))
        {
            // TODO: error handle
        }

        // TODO: del
        std::cout << "kqueue fd = " << kq_fd_ << std::endl;
    }
};

} // namespace cqnet
