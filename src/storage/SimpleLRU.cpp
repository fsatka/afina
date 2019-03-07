#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    std::size_t sumOfSize = SumOfSize(key, value);
    if (sumOfSize > _max_size)
        return false;

    while (sumOfSize > _free_size) {
        Delete(_lru_head->key);
    }

    _lru_index.find(key);
    
    //if (it != _lru_index.end()) {
    //    Delete(key);
    //}
    SimpleLRU::_InsertNode(key, value);
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

    lru_node* buff = &it->second.get();
    _free_size += SumOfSize(buff->key, buff->value);
    if(_lru_head.get() == _lru_tail){
        _lru_tail = nullptr;
        _lru_head.reset();
    }
    else if (buff == _lru_head.get()) {
        buff->next.swap(_lru_head);
        buff->next.reset();
    } else if (buff == _lru_tail) {
        _lru_tail = _lru_tail->prev;
        _lru_tail->next->prev = nullptr;
        _lru_tail->next.reset();
    } else {
        buff->next->prev = buff->prev;
        buff->next.swap(buff->prev->next);
        buff->prev = nullptr;
        buff->next.reset();
    }

    _lru_index.erase(key);
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
    lru_node* buff = new lru_node;
    buff->key = key;
    buff->value = value;
    if (_lru_tail == nullptr) {
        _lru_head.reset(buff);
        _lru_tail = _lru_head.get();
    } else {
        _lru_tail->next.reset(buff);
        _lru_tail->next->prev = _lru_tail;
        _lru_tail = _lru_tail->next.get();
    }
    _free_size -= SumOfSize(key, value);

    std::reference_wrapper<lru_node> ref_node(*buff);
    std::reference_wrapper<const std::string> ref_key(key);
    _lru_index.insert(std::pair<std::reference_wrapper<const std::string>,
                      std::reference_wrapper<lru_node>>(ref_key, ref_node));
}

std::size_t SimpleLRU::SumOfSize(const std::string &key, const std::string &value) const {
    return key.size() + value.size();
}

} // namespace Backend
} // namespace Afina