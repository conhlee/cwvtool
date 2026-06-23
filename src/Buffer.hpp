#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <Type.hpp>

#include <cstring>

class Buffer {
public:
    Buffer(void) {
        mData = NULL;
        mSize = 0;
    }
    Buffer(size_t size) {
        mData = new u64[(size + 7) / 8];
        mSize = size;

        memset(mData, 0x00, size);
    }
    Buffer(const Buffer &other) {
        if (other.check()) {
            mData = new u64[(other.mSize + 7) / 8];
            mSize = other.mSize;

            memcpy(mData, other.mData, mSize);
        }
        else {
            mData = NULL;
            mSize = 0;
        }
    }
    ~Buffer(void) {
       destroy();
    }

    Buffer &operator=(Buffer &&other) {
        if (this != &other) {
            destroy();
            mData = other.mData;
            mSize = other.mSize;

            other.mData = NULL;
            other.mSize = 0;
        }
        return *this;
    }

    void destroy(void) {
        if (mData != NULL) {
            delete[] static_cast<u64 *>(mData);
        }
        mData = NULL;
        mSize = 0;
    }

    bool check(void) const {
        return (mData != NULL) && (mSize != 0);
    }

    template <typename T = void>
    T *data(u32 offset = 0) {
        return reinterpret_cast<T *>(static_cast<u8 *>(mData) + offset);
    }
    template <typename T = void>
    const T *data(u32 offset = 0) const {
        return reinterpret_cast<const T *>(static_cast<u8 *>(mData) + offset);
    }

    size_t get_size(void) const {
        return mSize;
    }

    template <typename T = void>
    T *end(void) {
        return data<T>(mSize);
    }
    template <typename T = void>
    const T *end(void) const {
        return data<T>(mSize);
    }

private:
    void *mData;
    size_t mSize;
};

#endif // BUFFER_HPP
