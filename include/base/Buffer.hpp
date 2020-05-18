#pragma once

#include "base/Platform.hpp"

namespace cqnet {
namespace base {

template<typename T>
class Buffer final
{
private:
    T* data_{nullptr};
    size_t data_size_{0};
    size_t write_pos_{0};
    size_t read_pos_{0};

public:
    explicit Buffer(size_t buffer_size = 1024)
    {
        data_ = new T[buffer_size];
        if (data_ != nullptr)
        {
            data_size_ = buffer_size;
        }
    }

    ~Buffer()
    {
        delete[] data_;
    }

    void write(const T* data, size_t len)
    {
        if (get_writable_count() >= len)
        {
            memcpy(get_write_ptr(), data, len);
            add_write_pos(len);
            return;
        }

        size_t left_len = data_size_ - get_readable_count();
        if (left_len >= len)
        {
            adjust_to_head();
            write(data, len);
        }
        else
        {
            size_t need_len = len - left_len;
            grow(need_len);
            write(data, len);
        }
    }

    T* get_read_ptr()
    {
        if (read_pos_ < data_size_)
        {
            return data_ + read_pos_;
        }
        else
        {
            return nullptr;
        }
    }

    bool shift_n(size_t value)
    {
        size_t temp = read_pos_ + value;

        if (temp <= data_size_)
        {
            read_pos_ = temp;
            return true;
        }
        return false;
    }

    bool is_empty()
    {
        return write_pos_ == read_pos_;
    }

    size_t get_readable_count()
    {
        return write_pos_ - read_pos_;
    }

    size_t get_writable_count()
    {
        return data_size_ - write_pos_;
    }

    std::tuple<T*, size_t> get_read_ptr_with_readable_count()
    {
        return std::make_pair<T*, size_t>(get_read_ptr(), get_readable_count());
    }

    std::tuple<T*, size_t> get_write_ptr_with_writable_count()
    {
        return std::make_pair<T*, size_t>(get_write_ptr(), get_writable_count());
    }

    void reset()
    {
        read_pos_ = 0;
        write_pos_ = 0;
    }

private:
    void adjust_to_head()
    {
        size_t len = 0;

        if (read_pos_ <= 0)
        {
            return;
        }

        len = get_readable_count();

        if (len > 0)
        {
            memmove(data_, data_ + read_pos_, len);
        }

        read_pos_ = 0;
        write_pos_ = len;
    }

    size_t get_write_pos()
    {
        return write_pos_;
    }

    size_t get_read_pos()
    {
        return read_pos_;
    }

    void add_write_pos(size_t value)
    {
        size_t temp = write_pos_ + value;

        if (temp <= data_size_)
        {
            write_pos_ = temp;
        }
    }


    T* get_write_ptr()
    {
        if (write_pos_ < data_size_)
        {
            return data_ + write_pos_;
        }
        else
        {
            return nullptr;
        }
    }

    void grow(size_t len)
    {
        size_t n = data_size_ + len;
        T* d = new T[n];

        memcpy(d, data_, get_write_pos());
        data_size_ = n;
        delete[] data_;
        data_ = d;
    }
};

using CharBuffer = Buffer<char>;

} // namespace base
} // namespace cqnet
