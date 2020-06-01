#pragma once

#include "base/Platform.hpp"

namespace cqnet {

class NetAddr
{
public:
    using Ptr = std::shared_ptr<NetAddr>;

    Ptr static Create()
    {
        class make_shared_enabler : public NetAddr
        {
        public:
            make_shared_enabler()
                : NetAddr()
            {
            }
        };
        return std::make_shared<make_shared_enabler>();
    }

protected:
    NetAddr() = default;
    ~NetAddr() = default;
};

} // namespace cqnet
