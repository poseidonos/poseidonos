
#ifndef AIR_HASH_H
#define AIR_HASH_H

#include <cstdint>
#include <string>

namespace hash
{
template<typename K>
class HashNode
{
public:
    HashNode(K new_key, bool new_valid)
    : key(new_key),
      valid(new_valid)
    {
    }
    K key;
    bool valid;
};

template<typename K>
class HashMap
{
public:
    explicit HashMap(uint32_t new_capacity);
    virtual ~HashMap(void);
    virtual inline uint32_t
    InsertHashNode(K key)
    {
        if (capacity == GetHashSize())
        {
            return capacity;
        }
        uint32_t hash_index = _HashCode(key);
        uint32_t count = 0;
        while (hash_arr[hash_index]->key != key && hash_arr[hash_index]->valid == false)
        {
            count++;
            hash_index++;
            hash_index %= capacity;
        }
        if (hash_arr[hash_index]->valid == true)
        {
            size++;
        }
        hash_arr[hash_index]->key = key;
        hash_arr[hash_index]->valid = false;
        return hash_index;
    }
    virtual inline bool
    DeleteHashNode(K key)
    {
        uint32_t hash_index = _HashCode(key);
        uint32_t count = 0;
        while (count < capacity)
        {
            if (hash_arr[hash_index]->key == key)
            {
                hash_arr[hash_index]->key = -1;
                hash_arr[hash_index]->valid = true;
                size--;
                return true;
            }
            hash_index++;
            hash_index %= capacity;
            count++;
        }
        return false;
    }
    virtual inline uint32_t
    GetHashIndex(K key)
    {
        uint32_t hash_index = _HashCode(key);
        uint32_t count = 0;
        while (count < capacity)
        {
            if (hash_arr[hash_index]->key == key)
            {
                return hash_index;
            }
            hash_index++;
            hash_index %= capacity;
            count++;
        }
        return capacity;
    }
    virtual inline K
    GetHashKey(uint32_t hash_index)
    {
        if (hash_index < capacity)
        {
            return hash_arr[hash_index]->key;
        }
        return -1;
    }
    virtual inline uint32_t
    GetHashSize(void)
    {
        return size;
    }

private:
    uint32_t
    _HashCode(K key)
    {
        uint32_t x = (uint32_t)key;
        if (x < capacity)
        {
            return x;
        }
        x = ((x >> 16) ^ x) * 0x119de1f3;
        x = ((x >> 16) ^ x) * 0x119de1f3;
        x = (x >> 16) ^ x;
        return x % capacity;
    }

    HashNode<K>** hash_arr{nullptr};
    uint32_t capacity{0};
    uint32_t size{0};
};

} // namespace hash

#endif // AIR_HASH_H
