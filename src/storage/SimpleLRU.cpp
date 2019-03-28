#include "SimpleLRU.h"
#include <iostream>

inline std::size_t SumOfSize(const std::string &key, const std::string &value) { return key.size() + value.size(); }

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    std::size_t sumOfSize = SumOfSize(key, value);
    if (sumOfSize > _max_size)
        return false;

    lru_node *buff = nullptr;
    while (sumOfSize > _free_size) {
        if (_lru_head.get()->key != key) {
            _DeleteTail();
        } else {
            buff = _lru_head.get();
            _free_size += buff->value.size();
            sumOfSize -= buff->key.size();
            buff->value = "";
            _MoveNode(buff);
        }
    }

    if (buff == nullptr) {
        auto it = _lru_index.find(key);

        if (it != _lru_index.end()) {
            _free_size += it->second.get().value.size() - value.size();
            it->second.get().value = value;
            _MoveNode(_lru_head.get());
            return true;
        } else {
            _InsertNode(key, value);
            return true;
        }
    }

    _free_size -= value.size();
    buff->value = value;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if (it != _lru_index.end())
        return false;

    std::size_t sumOfSize = SumOfSize(key, value);
    if (sumOfSize > _max_size)
        return false;

    while (sumOfSize > _free_size) {
        _DeleteTail();
    }

    _InsertNode(key, value);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    std::size_t sumOfSize = SumOfSize(key, value);
    if (sumOfSize > _max_size)
        return false;

    lru_node *buff = nullptr;
    while (sumOfSize > _free_size) {
        if (_lru_head.get()->key != key)
            _DeleteTail();
        else {
            buff = _lru_head.get();
            _free_size += buff->value.size();
            sumOfSize -= buff->key.size();
            buff->value = "";
            _MoveNode(buff);
        }
    }

    if (buff == nullptr) {
        _free_size += it->second.get().value.size() - value.size();
        it->second.get().value = value;
        _MoveNode(_lru_head.get());
        return true;
    }

    _free_size -= value.size();
    buff->value = value;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    _lru_index.erase(it);

    lru_node *buff = &it->second.get();
    _free_size += SumOfSize(buff->key, buff->value);
    if (_lru_head.get() == _lru_tail) {
        _lru_tail = nullptr;
        _lru_head.reset();
    } else if (buff == _lru_head.get()) {
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
    
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    _MoveNode(&it->second.get());
    value = it->second.get().value;
    return true;
}

void SimpleLRU::_InsertNode(const std::string &key, const std::string &value) {
    lru_node *buff = new lru_node(key, value);
    if (_lru_tail == nullptr) {
        _lru_head.reset(buff);
        _lru_tail = _lru_head.get();
    } else {
        _lru_tail->next.reset(buff);
        buff->prev = _lru_tail;
        _lru_tail = buff;
    }
    _free_size -= SumOfSize(key, value);

    std::reference_wrapper<lru_node> ref_node(*_lru_tail);
    std::reference_wrapper<const std::string> ref_key(_lru_tail->key);
    _lru_index.insert(
        std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>>(ref_key, ref_node));
}

void SimpleLRU::_DeleteTail() {
    _free_size += SumOfSize(_lru_head->key, _lru_head->value);
    _lru_index.erase(_lru_head->key);
    _lru_head->next.swap(_lru_head);
    _lru_head->prev->next.reset();
    _lru_head->prev = nullptr;
}

void SimpleLRU::_MoveNode(lru_node *curr_node) {
    if (curr_node == _lru_tail) {
        return;
    }

    if (curr_node == _lru_head.get()) {
        curr_node->next->prev = nullptr;
        curr_node->next.swap(_lru_head);
        curr_node->next.swap(_lru_tail->next);
        curr_node->prev = _lru_tail;
        _lru_tail = curr_node;
        return;
    }

    curr_node->next->prev = curr_node->prev;
    curr_node->next.swap(curr_node->prev->next);
    _lru_tail->next.swap(curr_node->next);
    curr_node->prev = _lru_tail;
    _lru_tail = curr_node;
}

} // namespace Backend
} // namespace Afina