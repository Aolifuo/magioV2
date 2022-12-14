#ifndef MAGIO_CORE_FILE_H_
#define MAGIO_CORE_FILE_H_

#include <functional>
#include <system_error>
#include "magio-v3/core/noncopyable.h"

namespace magio {

template<typename>
class Coro;

class RandomAccessFile: Noncopyable {
    friend class File;

public:
    using Handle =
#ifdef  _WIN32
    void*;
#elif defined (__linux__)
    int;
#endif

    enum Openmode {
        ReadOnly  = 0b000001, 
        WriteOnly = 0b000010,
        ReadWrite = 0b000100,

        Create    = 0b001000,
        Truncate  = 0b010000,
        Append    = 0b100000
    };

    RandomAccessFile();

    RandomAccessFile(const char* path, int mode, int x = 0744);

    ~RandomAccessFile();

    RandomAccessFile(RandomAccessFile&& other) noexcept;

    RandomAccessFile& operator=(RandomAccessFile&& other) noexcept;

    void open(const char* path, int mode, int x = 0744);

    void cancel();

    void close();

#ifdef MAGIO_USE_CORO
    [[nodiscard]]
    Coro<size_t> read_at(size_t offset, char* buf, size_t len, std::error_code& ec);

    [[nodiscard]]
    Coro<size_t> write_at(size_t offset, const char* msg, size_t len, std::error_code& ec);

#endif
    void read_at(size_t offset, char* buf, size_t len, std::function<void(std::error_code, size_t)>&& completion_cb);

    void write_at(size_t offset, const char* msg, size_t len, std::function<void(std::error_code, size_t)>&& completion_cb);

    void sync_all();

    void sync_data();

    operator bool() const {
        return handle_ != (Handle)-1;
    }

private:
    void reset();

    Handle handle_;
    // only for win
    bool enable_app_;
};

class File: Noncopyable {
public:
    enum Openmode {
        ReadOnly  = 0b000001, 
        WriteOnly = 0b000010,
        ReadWrite = 0b000100,

        Create    = 0b001000,
        Truncate  = 0b010000,
        Append    = 0b100000
    };

    File();

    File(const char* path, int mode, int x = 0744);

    File(File&& other) noexcept;

    File& operator=(File&& other) noexcept;

    void open(const char* path, int mode, int x = 0744);

    void cancel();

    void close();

#ifdef MAGIO_USE_CORO
    [[nodiscard]]
    Coro<size_t> read(char* buf, size_t len, std::error_code& ec);
    
    [[nodiscard]]
    Coro<size_t> write(const char* buf, size_t len, std::error_code& ec);

#endif
    void read(char* buf, size_t len, std::function<void(std::error_code, size_t)>&& completion_cb);
    
    void write(const char* buf, size_t len, std::function<void(std::error_code, size_t)>&& completion_cb);

    void sync_all();

    void sync_data();

    operator bool() const {
        return file_;
    }

private:
    File(RandomAccessFile file);

    RandomAccessFile file_;
    size_t read_offset_ = 0;
    size_t write_offset_ = 0;
};

}

#endif