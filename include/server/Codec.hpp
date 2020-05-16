#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {

class NetConn;

class Codec
{
public:
    using Ptr = std::shared_ptr<Codec>;

    Ptr static Create()
    {
        class make_shared_enabler : public Codec
        {
        };
        return std::make_shared<make_shared_enabler>();
    }

    char* Encode(const NetConn* conn, char* buf)
    {
        return nullptr;
    }

    char* Decode(const NetConn* conn)
    {
        return nullptr;
    }

protected:
    Codec() = default;
};

} // namespace cqnet
