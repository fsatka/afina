#ifndef AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H
#define AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H

#include <map>
#include <mutex>
#include <string>

#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

/**
 * # SimpleLRU thread safe version
 *
 *
 */
class ThreadSafeSimplLRU : public SimpleLRU {
public:
    ThreadSafeSimplLRU(size_t max_size = 1024) : SimpleLRU(max_size) {}
    ~ThreadSafeSimplLRU() {}

    // see SimpleLRU.h
    bool Put(const std::string &key, const std::string &value) override {
        // TODO: sinchronization
        bool res;
        {
            std::lock_guard<std::mutex> lock(_lock_storage);
            res = SimpleLRU::Put(key, value); 
        }
        return res;
    }

    // see SimpleLRU.h
    bool PutIfAbsent(const std::string &key, const std::string &value) override {
        // TODO: sinchronization
        bool res;
        {
            std::lock_guard<std::mutex> lock(_lock_storage);
            res = SimpleLRU::Set(key, value);
        }
        return res;
    }

    // see SimpleLRU.h
    bool Set(const std::string &key, const std::string &value) override {
        // TODO: sinchronization
        bool res;
        {
            std::lock_guard<std::mutex> lock(_lock_storage);
            res = SimpleLRU::Set(key, value);
        }
        return res;
    }

    // see SimpleLRU.h
    bool Delete(const std::string &key) override {
        // TODO: sinchronization
        bool res;
        {
            std::lock_guard<std::mutex> lock(_lock_storage);
            SimpleLRU::Delete(key);
        }
        return res;
    }

    // see SimpleLRU.h
    bool Get(const std::string &key, std::string &value) const override {
        // TODO: sinchronization
        bool res;
        {
            std::lock_guard<std::mutex> lock(_lock_storage);
            res = SimpleLRU::Get(key, value);
        }

        return res;
    }

private:
    // TODO: sinchronization primitives
    std::mutex _lock_storage;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H
