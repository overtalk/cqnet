#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {
namespace base {

class WaitGroup : public Noncopyable
{
private:
    std::mutex mMutex;
    std::atomic<int> mCounter{0};
    std::condition_variable mCond;

public:
    typedef std::shared_ptr<WaitGroup> Ptr;

    static Ptr Create()
    {
        struct make_shared_enabler : public WaitGroup
        {
        };
        return std::make_shared<make_shared_enabler>();
    }

public:
    void add(int i = 1)
    {
        mCounter += i;
    }

    void done()
    {
        mCounter--;
        mCond.notify_all();
    }

    void wait()
    {
        std::unique_lock<std::mutex> l(mMutex);
        mCond.wait(l, [&] { return mCounter <= 0; });
    }

    template<class Rep, class Period>
    void wait(const std::chrono::duration<Rep, Period>& timeout)
    {
        std::unique_lock<std::mutex> l(mMutex);
        mCond.wait_for(l, timeout, [&] { return mCounter <= 0; });
    }

private:
    WaitGroup() {}

    virtual ~WaitGroup() = default;
};
} // namespace base
} // namespace cqnet