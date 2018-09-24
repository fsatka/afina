#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) { 
    if(key.size()+value.size() > _max_size) {
        return false;
    }
    
    while(key.size()+value.size() > _free_space) {
        SimpleLRU::_Delete_tail_node();
    }

    lru_node* buff_node = new lru_node;
    buff_node->key = key;
    buff_node->value = value;

    _free_space -= key.size()+value.size();
    SimpleLRU::_Insert_in_list(buff_node);
    SimpleLRU::_Insert_in_storage(buff_node);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) { 
    if(_lru_index.count(key) != 0) {
        return false; 
    }

    return SimpleLRU::Put(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if(_lru_index.count(key)==0) {
        return false;
    }
    return SimpleLRU::Put(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    if(_lru_index.count(key)==0){
        return false;
    }

    lru_node* buff = &_lru_index.at(key).get();

    if(buff == _lru_tail.get()) {
        return SimpleLRU::_Delete_tail_node();
    }

    size_t dm = key.size() + buff->value.size();
    if(buff == _lru_head){
        _lru_head = buff->prev;
        if(_lru_head != nullptr){
            _lru_head->next.reset(nullptr);
        }
    }
    else{
        buff->next.get()->prev = buff->prev;
        buff->prev->next.reset(buff->next.get());
    }

    _lru_index.erase(key);
    _free_space += dm;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) const { 
    if(_lru_index.count(key) == 0) {
        return false;
    }

    lru_node* buff = &_lru_index.at(key).get();

    value = buff->value;
    if(buff == _lru_head) {
        return true;
    }

    if(buff->prev == nullptr) {
        _lru_tail.swap(buff->next);
        _lru_tail.get()->prev = nullptr;
        buff->prev = _lru_head;
        _lru_head->next.swap(buff->next);
        return true;
    }

    buff->next.get()->prev = buff->prev;
    buff->prev->next.swap(buff->next);
    buff->prev = _lru_head;
    _lru_head->next.swap(buff->next);

    return true;
}

//Inner methods
bool SimpleLRU::_Delete_tail_node() {
    lru_node* buff = _lru_tail.get();
    if(buff == nullptr) {
        return false;
    }

    _lru_index.erase(buff->key);
    size_t dm = buff->key.size() + buff->value.size();

    _lru_tail.swap(buff->next);
    _lru_tail.get()->prev = nullptr;
    buff->next.reset(nullptr);
    _free_space += dm;
    return true;
}

void SimpleLRU::_Insert_in_list(lru_node* const _node) {

    if(_lru_head == nullptr) {
        _lru_head = _node;
        _lru_tail.reset(_node);
        return;
    }

    _lru_head->next.reset(_node);
    _node->prev = _lru_head;
    _lru_head = _node;
}

void SimpleLRU::_Insert_in_storage(lru_node* const _node) {
    std::reference_wrapper<lru_node> ref_node(*_node);

    if(_lru_index.count(_node->key)==0) {
    std::reference_wrapper<const std::string> ref_key(_node->key);
    _lru_index.insert(std::pair<std::reference_wrapper<const std::string>,
                      std::reference_wrapper<lru_node>>(ref_key, ref_node));
    }
    else {
        _lru_index.at(_node->key) = ref_node;
    }
    
}

} // namespace Backend
} // namespace Afina
