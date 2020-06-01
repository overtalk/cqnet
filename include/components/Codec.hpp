#pragma once

#include "base/Platform.hpp"
#include "base/SDS.hpp"
#include "Interface.hpp"

namespace cqnet {

class Codec : public ICodec
{
    base::SDS::Ptr Encode(INetConn* conn, base::SDS::Ptr sds) override
    {
        return std::move(sds);
    }

    base::SDS::Ptr Decode(INetConn* conn_ptr) override
    {
        auto buf = conn_ptr->ReadBuffer();
        char* data = std::get<0>(buf);
        int size = std::get<1>(buf);
        auto temp = base::SDS::Create(data, size);
        conn_ptr->ShiftN(size);
        return std::move(temp);
    }
};

} // namespace cqnet
