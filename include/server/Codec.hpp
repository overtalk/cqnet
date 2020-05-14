#pragma once

#include "base/NonCopyable.hpp"

namespace cqnet {

class NetConn;

class Codec
{
public:
    char* Encode(const NetConn* conn, char* buf)
    {
        return nullptr;
    }

    char* Decode(const NetConn* conn)
    {
        return nullptr;
    }
};

} // namespace cqnet
