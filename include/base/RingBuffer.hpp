#pragma once

#include "base/Platform.hpp"

namespace cqnet {
namespace base {

template<typename T>
class RingBuffer
{
private:
    T* data_;
    size_t size_;
    size_t mask_;
    size_t read_pos_{0};  // next position to read
    size_t write_pos_{0}; // next position to write
    bool is_empty_{true};

public:
    explicit RingBuffer(size_t buffer_size = 1024)
    {
        if (buffer_size == 0)
        {
            return;
        }

        data_ = new char[buffer_size];
        if (data_ != nullptr)
        {
            size_ = buffer_size;
            mask_ = buffer_size - 1;
        }
    }

    ~RingBuffer()
    {
        do
        {
            if ((data_) != nullptr)
            {
                char* t_ptr = static_cast<char*>((data_));
                delete[] t_ptr;
                t_ptr = nullptr;
            }
        } while (false);
    }

    bool IsEmpty()
    {
        return is_empty_;
    }

    // read
    // if return -1, error occur
    size_t LazyReadN(T* data, size_t len)
    {
        return Read(data, len);
    }

    size_t LazyReadAll(T* data)
    {
        return LazyReadN(data, Size());
    }

    size_t ReadN(T* data, size_t len)
    {
        size_t ret = Read(data, len);
        if (ret > 0)
        {
            ShiftN(ret);
        }

        return ret;
    }

    size_t ReadAll(T* data)
    {
        return ReadN(data, Size());
    }

    // write data
    size_t Write(T* data, size_t size)
    {
        if (size <= 0)
        {
            return size;
        }

        size_t available = AvailableSize();
        if (size > available)
        {
            Grow(size - available);
        }

        if (write_pos_ >= read_pos_)
        {
            size_t l1 = size_ - write_pos_;
            if (l1 >= size)
            {
                memcpy(data_ + write_pos_, data, size);
                write_pos_ += size;
            }
            else
            {
                memcpy(data_ + write_pos_, data, l1);
                size_t l2 = size - l1;
                memcpy(data_ + write_pos_ + l1, data + l1, l2);
                write_pos_ = l2;
            }
        }
        else
        {
            memcpy(data_ + write_pos_, data, size);
            write_pos_ += size;
        }

        if (write_pos_ == size_)
        {
            write_pos_ = 0;
        }

        is_empty_ = false;
        return size;
    }

    void ShiftN(size_t n)
    {
        if (n <= 0)
        {
            return;
        }

        if (n < Size())
        {
            read_pos_ = (read_pos_ + n) & mask_;
            if (read_pos_ == write_pos_)
            {
                is_empty_ = true;
            }
        }
        else
        {
            Reset();
        }
    }

    size_t Size()
    {
        if (read_pos_ == write_pos_)
        {
            if (is_empty_)
            {
                return 0;
            }
            return size_;
        }

        if (write_pos_ > read_pos_)
        {
            return write_pos_ - read_pos_;
        }
        return size_ - (write_pos_ - read_pos_);
    }

    size_t AvailableSize()
    {
        if (read_pos_ == write_pos_)
        {
            if (is_empty_)
            {
                return size_;
            }
            return 0;
        }

        if (write_pos_ < read_pos_)
        {
            return read_pos_ - write_pos_;
        }
        return size_ - write_pos_ + read_pos_;
    }

    void Reset()
    {
        read_pos_ = 0;
        write_pos_ = 0;
        is_empty_ = true;
    }

private:
    size_t read(T* data, size_t read_size)
    {
        if (read_size <= 0)
        {
            return read_size;
        }

        if (is_empty_)
        {
            return 0;
        }

        // n defines the real read data size
        size_t n = Size();
        if (n > read_size)
        {
            n = read_size;
        }

        // write pos is behind the read pos
        if (write_pos_ > read_pos_)
        {
            memcpy(data, data_ + read_pos_, n);
        }
        else
        {
            size_t tail = size_ - read_pos_;
            size_t head = n - tail;
            memcpy(data, data_ + read_pos_, tail);
            memcpy(data + tail, data_, head);
        }
        return n;
    }

    void grow(size_t len)
    {
        size_t n = size_ + len;
        T* temp = new T[n];
        Read(temp, size_);
        size_ = n;
        mask_ = n - 1;
        delete data_;
        data_ = temp;
    }
};

} // namespace base
} // namespace cqnet
