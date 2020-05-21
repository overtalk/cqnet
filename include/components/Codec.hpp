#pragma once

#include "Interface.hpp"

namespace cqnet {

class Codec : public ICodec
{
    char* Encode(std::shared_ptr<NetConn> conn, char* buf)
    {
        std::cout << "Encode" << std::endl;
        return "Encode";
    }

    char* Decode(std::shared_ptr<NetConn> conn)
    {
        std::cout << "Decode" << std::endl;
        return "Decode";
    }
};

} // namespace cqnet
