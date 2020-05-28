#pragma once

#include "base/Platform.hpp"
#include "base/SDS.hpp"
#include "Interface.hpp"

namespace cqnet {

class Codec : public ICodec
{
    char* Encode(std::shared_ptr<INetConn> conn, char* buf) override
    {
        std::cout << "Encode" << std::endl;
        return "Encode";
    }

    base::SDS::Ptr Decode(std::shared_ptr<INetConn> conn_ptr) override
    {
        auto buf = conn_ptr->ReadBuffer();
        char* data = std::get<0>(buf);
        int size = std::get<1>(buf);

        auto temp = base::SDS::Create(std::get<0>(buf), std::get<1>(buf));

        conn_ptr->ShiftN(size);
        return std::move(temp);
    }
};

} // namespace cqnet
