
#include "src/lib/Hash.h"

template<typename K>
hash::HashMap<K>::HashMap(uint32_t new_capacity)
{
    capacity = new_capacity;
    hash_arr = new HashNode<K>*[capacity];
    for (uint32_t i = 0; i < capacity; i++)
    {
        hash_arr[i] = new HashNode<K>(-1, true);
    }
}

template<typename K>
hash::HashMap<K>::~HashMap(void)
{
    for (uint32_t i = 0; i < capacity; i++)
    {
        if (hash_arr[i] != nullptr)
        {
            delete hash_arr[i];
            hash_arr[i] = nullptr;
        }
    }
    delete[] hash_arr;
}

// explicit instantiation
template class hash::HashMap<uint64_t>;
