#pragma once

#include "base/Platform.hpp"

namespace cqnet {
namespace base {

class AsyncJobQueue
{
public:
    using AsyncJob = std::function<bool()>;

private:
    std::mutex mut_;
    std::vector<AsyncJob> async_jobs_;

public:
    size_t Push(const AsyncJob& job)
    {
        mut_.lock();
        async_jobs_.push_back(job);
        mut_.unlock();
        return async_jobs_.size();
    }

    bool FroEach()
    {
        mut_.lock();
        std::vector<AsyncJob> old_jobs = async_jobs_;
        async_jobs_ = std::vector<AsyncJob>();
        mut_.unlock();

        for (int i = 0; i < old_jobs.size(); ++i)
        {
            if (!old_jobs[i]())
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace base
} // namespace cqnet