#pragma once

#include "Interface.hpp"

namespace cqnet {

class Codec : public ICodec
{
    char* Encode(std::shared_ptr<INetConn> conn, char* buf) override
    {
        std::cout << "Encode" << std::endl;
        return "Encode";
    }

    char* Decode(std::shared_ptr<INetConn> conn_ptr) override
    {
        auto buf = conn_ptr->ReadBuffer();
        char* data = std::get<0>(buf);
        int size = std::get<1>(buf);
        std::cout << size << "  " << data << std::endl;
        conn_ptr->ShiftN(size);

        std::cout << "Decode" << std::endl;
        return "Decode";
    }
};

} // namespace cqnet
