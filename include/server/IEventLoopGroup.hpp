#pragma once

#include "base/Platform.hpp"

namespace cqnet {

class EventLoop;

using IterateFunc = std::function<bool(int, const EventLoop&)>;

class IEventLoopGroup
{
public:
    IEventLoopGroup() = default;
    virtual ~IEventLoopGroup() = default;

private:
    virtual void Register(const EventLoop& el) = 0;
    virtual const EventLoop& Next() = 0;
    virtual void Iterate(const IterateFunc& func) = 0;
    virtual const int Len() = 0;
};

} // namespace cqnet
