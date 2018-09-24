#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {

/**
 * # Map based implementation
 * That is NOT thread safe implementaiton!!
 */
class SimpleLRU : public Afina::Storage {
public:
    SimpleLRU(size_t max_size = 1024) : 
    _max_size(max_size), 
    _free_space(max_size),
    _lru_head(nullptr)
    {}

    ~SimpleLRU() {
        _lru_index.clear();// TODO: Here is stack overflow
        _lru_tail.reset();

    }

    // Implements Afina::Storage interface
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) const override;
     


private:
    // LRU cache node
    using lru_node = struct lru_node {
        std::string key;
        std::string value;
        lru_node* prev = nullptr;
        std::unique_ptr<lru_node> next;
        ~lru_node() {
        for (std::unique_ptr<lru_node> current = std::move(next);
             current;
             current = std::move(current->next));
        }
    };

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;
    std::size_t _free_space;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    mutable lru_node* _lru_head;
    mutable std::unique_ptr<lru_node> _lru_tail;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, 
                    std::reference_wrapper<lru_node>, 
                    std::less<std::string>> _lru_index;

    //Inner methods
    bool _Delete_tail_node();
    void _Insert_in_list(lru_node* const _node);
    void _Insert_in_storage(lru_node* const _node);

};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
