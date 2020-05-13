#pragma once

namespace cqnet {
namespace base {

class Noncopyable
{
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
    Noncopyable(Noncopyable&&) = delete;
    Noncopyable& operator=(Noncopyable&&) = delete;
};

} // namespace base
} // namespace cqnet