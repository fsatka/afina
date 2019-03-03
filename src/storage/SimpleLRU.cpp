#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    std::size_t sumOfSize = SumOfSize(key, value);
    if (sumOfSize > _max_size)
        return false;

    while (sumOfSize > _free_size) {
        _free_size += SumOfSize(_lru_head->key, _lru_head->value);
        Delete(_lru_head->key);
    }

    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        Delete(key);
    }
    _InsertNode(key, value);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        _InsertNode(key, value);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        Put(key, value);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    if (&it->second.get() == _lru_head.get()) {
        it->second.get().next.swap(_lru_head);
        it->second.get().next.reset();
        _lru_head->prev = nullptr;
    } else if (&it->second.get() == _lru_tail) {
        _lru_tail = _lru_tail->prev;
        _lru_tail->next->prev = nullptr;
        _lru_tail->next.reset();
    } else {
        it->second.get().next->prev = it->second.get().prev;
        it->second.get().next.swap(it->second.get().prev->next);
        it->second.get().prev = nullptr;
        it->second.get().next.reset();
    }

    _lru_index.erase(it);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) const {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    value = it->second.get().value;
    return true;
}

void SimpleLRU::_InsertNode(const std::string &key, const std::string &value) {
    _lru_tail->next.reset(new lru_node());
    _lru_tail->next->prev = _lru_tail;
    _lru_tail = _lru_tail->next.get();
    _lru_tail->key = key;
    _lru_tail->value = value;
}

std::size_t SimpleLRU::SumOfSize(const std::string &key, const std::string &value) const{
    return key.size()+value.size();
}

} // namespace Backend
} // namespace Afina