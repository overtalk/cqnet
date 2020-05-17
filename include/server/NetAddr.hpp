#pragma once

#include "base/Platform.hpp"

namespace cqnet {

class NetAddr
{
public:
    using Ptr = std::shared_ptr<NetAddr>;

    Ptr static Create()
    {
        return std::make_shared<NetAddr>();
    }

protected:
    NetAddr();
    ~NetAddr();
};

} // namespace cqnet
