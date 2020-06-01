#pragma once

#include "base/Platform.hpp"
#include "base/NonCopyable.hpp"

namespace cqnet {
namespace base {

class SDS : public NonCopyable
{
private:
    char* data{nullptr};
    int size{0};

protected:
    SDS(const char* d, int s)
        : data(new char[size])
        , size(s)
    {
        memcpy(data, d, size);
    }

    ~SDS()
    {
        std::cout << "SDS destroy!" << std::endl;
        delete[] data;
    }

public:
    using Ptr = std::shared_ptr<SDS>;

    Ptr static Create(const char* d, int s)
    {
        class make_shared_enabler : public SDS
        {
        public:
            make_shared_enabler(const char* d, int s)
                : SDS(d, s)
            {
            }
        };

        return std::make_shared<make_shared_enabler>(d, s);
    }

    char* GetData()
    {
        return data;
    }

    int GetSize() const
    {
        return size;
    }
};

} // namespace base
} // namespace cqnet
