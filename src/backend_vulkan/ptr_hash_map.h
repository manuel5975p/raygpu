#ifndef HASHMAP_SET_AND_VECTOR_H
#define HASHMAP_SET_AND_VECTOR_H

#include <assert.h> // Added assert for internal checks
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef PHM_INLINE_CAPACITY
#define PHM_INLINE_CAPACITY 3
#endif

#ifndef PHM_INITIAL_HEAP_CAPACITY
#define PHM_INITIAL_HEAP_CAPACITY 8
#endif

#ifndef PHM_LOAD_FACTOR_NUM
#define PHM_LOAD_FACTOR_NUM 3
#endif

#ifndef PHM_LOAD_FACTOR_DEN
#define PHM_LOAD_FACTOR_DEN 4
#endif

#ifndef PHM_HASH_MULTIPLIER
#define PHM_HASH_MULTIPLIER 0x9E3779B97F4A7C15ULL
#endif

#define PHM_EMPTY_SLOT_KEY NULL

#define DEFINE_PTR_HASH_MAP(SCOPE, Name, ValueType)                                                                              \
                                                                                                                                 \
    typedef struct Name##_kv_pair {                                                                                              \
        void *key;                                                                                                               \
        ValueType value;                                                                                                         \
    } Name##_kv_pair;                                                                                                            \
                                                                                                                                 \
    typedef struct Name {                                                                                                        \
        uint64_t current_size;                                                                                                   \
        bool has_null_key;                                                                                                       \
        ValueType null_value;                                                                                                    \
        Name##_kv_pair *table;                                                                                                   \
        union {                                                                                                                  \
            uint64_t current_capacity;                                                                                           \
            Name##_kv_pair inline_buffer[PHM_INLINE_CAPACITY];                                                                   \
        };                                                                                                                       \
    } Name;                                                                                                                      \
                                                                                                                                 \
    static inline uint64_t Name##_hash_key(void *key) {                                                                          \
        assert(key != NULL);                                                                                                     \
        return ((uintptr_t)key) * PHM_HASH_MULTIPLIER;                                                                           \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_grow(Name *map);                                                                                          \
                                                                                                                                 \
    SCOPE void Name##_init(Name *map) {                                                                                          \
        map->current_size = 0;                                                                                                   \
        map->has_null_key = false;                                                                                               \
        map->table = map->inline_buffer;                                                                                         \
        memset(map->inline_buffer, 0, sizeof(map->inline_buffer));                                                               \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_insert_entry_heap(Name##_kv_pair *table, uint64_t capacity, void *key, ValueType value) {                 \
        assert(key != NULL && key != PHM_EMPTY_SLOT_KEY);                                                                        \
        assert(capacity > 0 && (capacity & (capacity - 1)) == 0);                                                                \
        uint64_t cap_mask = capacity - 1;                                                                                        \
        uint64_t h = Name##_hash_key(key);                                                                                       \
        uint64_t index = h & cap_mask;                                                                                           \
        while (1) {                                                                                                              \
            Name##_kv_pair *slot = &table[index];                                                                                \
            if (slot->key == PHM_EMPTY_SLOT_KEY) {                                                                               \
                slot->key = key;                                                                                                 \
                slot->value = value;                                                                                             \
                return;                                                                                                          \
            }                                                                                                                    \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    static inline Name##_kv_pair *Name##_find_slot_heap(Name *map, void *key) {                                                  \
        assert(key != NULL && key != PHM_EMPTY_SLOT_KEY);                                                                        \
        assert(map->table != map->inline_buffer && map->current_capacity > 0);                                                   \
        uint64_t cap_mask = map->current_capacity - 1;                                                                           \
        uint64_t h = Name##_hash_key(key);                                                                                       \
        uint64_t index = h & cap_mask;                                                                                           \
        while (1) {                                                                                                              \
            Name##_kv_pair *slot = &map->table[index];                                                                           \
            if (slot->key == PHM_EMPTY_SLOT_KEY || slot->key == key) {                                                           \
                return slot;                                                                                                     \
            }                                                                                                                    \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_grow(Name *map) {                                                                                         \
        bool was_inline = (map->table == map->inline_buffer);                                                                    \
        uint64_t old_non_null_size = map->current_size;                                                                          \
        uint64_t old_capacity = was_inline ? PHM_INLINE_CAPACITY : map->current_capacity;                                        \
        Name##_kv_pair *old_table_ptr = map->table;                                                                              \
        uint64_t new_capacity = was_inline ? PHM_INITIAL_HEAP_CAPACITY : old_capacity * 2;                                       \
        if (new_capacity < PHM_INITIAL_HEAP_CAPACITY) {                                                                          \
            new_capacity = PHM_INITIAL_HEAP_CAPACITY;                                                                            \
        }                                                                                                                        \
        if (new_capacity > 0 && (new_capacity & (new_capacity - 1)) != 0) {                                                      \
            uint64_t pow2 = 1;                                                                                                   \
            while (pow2 < new_capacity)                                                                                          \
                pow2 <<= 1;                                                                                                      \
            new_capacity = pow2;                                                                                                 \
        }                                                                                                                        \
        if (new_capacity == 0)                                                                                                   \
            return;                                                                                                              \
        Name##_kv_pair *new_table = (Name##_kv_pair *)calloc(new_capacity, sizeof(Name##_kv_pair));                              \
        if (!new_table)                                                                                                          \
            return;                                                                                                              \
        if (was_inline) {                                                                                                        \
            uint64_t count = 0;                                                                                                  \
            for (uint64_t i = 0; i < PHM_INLINE_CAPACITY; ++i) {                                                                 \
                if (old_table_ptr[i].key != PHM_EMPTY_SLOT_KEY) {                                                                \
                    Name##_insert_entry_heap(new_table, new_capacity, old_table_ptr[i].key, old_table_ptr[i].value);             \
                    count++;                                                                                                     \
                    if (count == old_non_null_size)                                                                              \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        } else {                                                                                                                 \
            if (old_capacity > 0) {                                                                                              \
                uint64_t count = 0;                                                                                              \
                for (uint64_t i = 0; i < old_capacity; ++i) {                                                                    \
                    Name##_kv_pair *entry = &old_table_ptr[i];                                                                   \
                    if (entry->key != PHM_EMPTY_SLOT_KEY) {                                                                      \
                        Name##_insert_entry_heap(new_table, new_capacity, entry->key, entry->value);                             \
                        count++;                                                                                                 \
                        if (count == old_non_null_size)                                                                          \
                            break;                                                                                               \
                    }                                                                                                            \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
        if (!was_inline && old_table_ptr != NULL) {                                                                              \
            free(old_table_ptr);                                                                                                 \
        }                                                                                                                        \
        map->table = new_table;                                                                                                  \
        map->current_capacity = new_capacity;                                                                                    \
        map->current_size = old_non_null_size;                                                                                   \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_put(Name *map, void *key, ValueType value) {                                                               \
        if (key == NULL) {                                                                                                       \
            map->has_null_key = true;                                                                                            \
            map->null_value = value;                                                                                             \
            return;                                                                                                              \
        }                                                                                                                        \
        assert(key != PHM_EMPTY_SLOT_KEY);                                                                                       \
        if (map->table == map->inline_buffer) {                                                                                  \
            uint64_t first_empty_slot = PHM_INLINE_CAPACITY;                                                                     \
            for (uint64_t i = 0; i < PHM_INLINE_CAPACITY; ++i) {                                                                 \
                if (map->inline_buffer[i].key == key) {                                                                          \
                    map->inline_buffer[i].value = value;                                                                         \
                    return;                                                                                                      \
                }                                                                                                                \
                if (map->inline_buffer[i].key == PHM_EMPTY_SLOT_KEY && first_empty_slot == PHM_INLINE_CAPACITY) {                \
                    first_empty_slot = i;                                                                                        \
                }                                                                                                                \
            }                                                                                                                    \
            if (first_empty_slot < PHM_INLINE_CAPACITY) {                                                                        \
                map->inline_buffer[first_empty_slot].key = key;                                                                  \
                map->inline_buffer[first_empty_slot].value = value;                                                              \
                map->current_size++;                                                                                             \
                return;                                                                                                          \
            } else {                                                                                                             \
                Name##_grow(map);                                                                                                \
                if (map->table == map->inline_buffer)                                                                            \
                    return;                                                                                                      \
            }                                                                                                                    \
        }                                                                                                                        \
        assert(map->table != map->inline_buffer);                                                                                \
        bool needs_grow = (map->current_capacity == 0);                                                                          \
        if (!needs_grow && map->current_capacity > 0) {                                                                          \
            needs_grow = (map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM;           \
        }                                                                                                                        \
        if (needs_grow) {                                                                                                        \
            Name##_grow(map);                                                                                                    \
            if (map->current_capacity == 0)                                                                                      \
                return;                                                                                                          \
        }                                                                                                                        \
        Name##_kv_pair *slot = Name##_find_slot_heap(map, key);                                                                  \
        if (slot->key == PHM_EMPTY_SLOT_KEY) {                                                                                   \
            slot->key = key;                                                                                                     \
            slot->value = value;                                                                                                 \
            map->current_size++;                                                                                                 \
        } else {                                                                                                                 \
            assert(slot->key == key);                                                                                            \
            slot->value = value;                                                                                                 \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE ValueType *Name##_get(Name *map, void *key) {                                                                          \
        if (key == NULL) {                                                                                                       \
            return map->has_null_key ? &map->null_value : NULL;                                                                  \
        }                                                                                                                        \
        assert(key != PHM_EMPTY_SLOT_KEY);                                                                                       \
        if (map->table == map->inline_buffer) {                                                                                  \
            for (uint64_t i = 0; i < PHM_INLINE_CAPACITY; ++i) {                                                                 \
                if (map->inline_buffer[i].key == key) {                                                                          \
                    return &map->inline_buffer[i].value;                                                                         \
                }                                                                                                                \
            }                                                                                                                    \
            return NULL;                                                                                                         \
        }                                                                                                                        \
        if (map->current_capacity == 0)                                                                                          \
            return NULL;                                                                                                         \
        Name##_kv_pair *slot = Name##_find_slot_heap(map, key);                                                                  \
        return (slot->key == key) ? &slot->value : NULL;                                                                         \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_for_each(Name *map, void (*callback)(void *key, ValueType *value, void *user_data), void *user_data) {     \
        if (map->has_null_key) {                                                                                                 \
            callback(NULL, &map->null_value, user_data);                                                                         \
        }                                                                                                                        \
        if (map->table == map->inline_buffer) {                                                                                  \
            uint64_t count = 0;                                                                                                  \
            for (uint64_t i = 0; i < PHM_INLINE_CAPACITY; ++i) {                                                                 \
                if (map->inline_buffer[i].key != PHM_EMPTY_SLOT_KEY) {                                                           \
                    callback(map->inline_buffer[i].key, &map->inline_buffer[i].value, user_data);                                \
                    count++;                                                                                                     \
                    if (count == map->current_size)                                                                              \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        } else {                                                                                                                 \
            if (map->current_capacity > 0) {                                                                                     \
                uint64_t count = 0;                                                                                              \
                for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                           \
                    if (map->table[i].key != PHM_EMPTY_SLOT_KEY) {                                                               \
                        callback(map->table[i].key, &map->table[i].value, user_data);                                            \
                        count++;                                                                                                 \
                        if (count == map->current_size)                                                                          \
                            break;                                                                                               \
                    }                                                                                                            \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_free(Name *map) {                                                                                          \
        if (map->table != map->inline_buffer && map->table != NULL) {                                                            \
            free(map->table);                                                                                                    \
        }                                                                                                                        \
        Name##_init(map);                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_move(Name *dest, Name *source) {                                                                           \
        if (dest == source) {                                                                                                    \
            return;                                                                                                              \
        }                                                                                                                        \
                                                                                                                                 \
        dest->current_size = source->current_size;                                                                               \
        dest->has_null_key = source->has_null_key;                                                                               \
        dest->null_value = source->null_value;                                                                                   \
        if (source->table == source->inline_buffer) {                                                                            \
            memcpy(dest->inline_buffer, source->inline_buffer, sizeof(source->inline_buffer));                                   \
            dest->table = dest->inline_buffer;                                                                                   \
        } else {                                                                                                                 \
            dest->table = source->table;                                                                                         \
            dest->current_capacity = source->current_capacity;                                                                   \
        }                                                                                                                        \
                                                                                                                                 \
        Name##_init(source);                                                                                                     \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                                                                     \
        Name##_init(dest);                                                                                                       \
        dest->has_null_key = source->has_null_key;                                                                               \
        if (source->has_null_key) {                                                                                              \
            dest->null_value = source->null_value;                                                                               \
        }                                                                                                                        \
        dest->current_size = source->current_size;                                                                               \
                                                                                                                                 \
        if (source->table == source->inline_buffer) {                                                                            \
            memcpy(dest->inline_buffer, source->inline_buffer, sizeof(source->inline_buffer));                                   \
            dest->table = dest->inline_buffer;                                                                                   \
        } else {                                                                                                                 \
            if (source->current_capacity > 0) {                                                                                  \
                dest->table = (Name##_kv_pair *)calloc(source->current_capacity, sizeof(Name##_kv_pair));                        \
                if (!dest->table) {                                                                                              \
                    Name##_init(dest);                                                                                           \
                    return;                                                                                                      \
                }                                                                                                                \
                memcpy(dest->table, source->table, source->current_capacity * sizeof(Name##_kv_pair));                           \
                dest->current_capacity = source->current_capacity;                                                               \
            } else {                                                                                                             \
                dest->table = dest->inline_buffer;                                                                               \
            }                                                                                                                    \
        }                                                                                                                        \
    }

#define DEFINE_PTR_HASH_SET(SCOPE, Name)                                                                                         \
                                                                                                                                 \
    typedef struct Name {                                                                                                        \
        uint64_t current_size;                                                                                                   \
        bool has_null_element;                                                                                                   \
        void **table;                                                                                                            \
        union {                                                                                                                  \
            uint64_t current_capacity;                                                                                           \
            void *inline_buffer[PHM_INLINE_CAPACITY];                                                                            \
        };                                                                                                                       \
    } Name;                                                                                                                      \
                                                                                                                                 \
    static inline uint64_t Name##_hash_key(void *key) { return (uintptr_t)key * PHM_HASH_MULTIPLIER; }                           \
                                                                                                                                 \
    static void Name##_grow(Name *set);                                                                                          \
                                                                                                                                 \
    SCOPE void Name##_init(Name *set) {                                                                                          \
        set->current_size = 0;                                                                                                   \
        set->has_null_element = false;                                                                                           \
        set->table = set->inline_buffer;                                                                                         \
        memset(set->inline_buffer, 0, sizeof(set->inline_buffer));                                                               \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_insert_key(void **table, uint64_t capacity, void *key) {                                                  \
        assert(key != PHM_EMPTY_SLOT_KEY);                                                                                       \
        assert(capacity > 0 && (capacity & (capacity - 1)) == 0);                                                                \
        uint64_t cap_mask = capacity - 1;                                                                                        \
        uint64_t h = Name##_hash_key(key);                                                                                       \
        uint64_t index = h & cap_mask;                                                                                           \
        while (1) {                                                                                                              \
            void **slot_ptr = &table[index];                                                                                     \
            if (*slot_ptr == PHM_EMPTY_SLOT_KEY) {                                                                               \
                *slot_ptr = key;                                                                                                 \
                return;                                                                                                          \
            }                                                                                                                    \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_grow(Name *set) {                                                                                         \
        bool is_inline = (set->table == set->inline_buffer);                                                                     \
        uint64_t old_non_null_size = set->current_size;                                                                          \
        uint64_t old_capacity = is_inline ? PHM_INLINE_CAPACITY : set->current_capacity;                                         \
        void **old_table_ptr = set->table;                                                                                       \
                                                                                                                                 \
        uint64_t new_capacity = (old_capacity < PHM_INITIAL_HEAP_CAPACITY) ? PHM_INITIAL_HEAP_CAPACITY : old_capacity * 2;       \
        if (new_capacity < PHM_INITIAL_HEAP_CAPACITY) {                                                                          \
            new_capacity = PHM_INITIAL_HEAP_CAPACITY;                                                                            \
        }                                                                                                                        \
        if (new_capacity > 0 && (new_capacity & (new_capacity - 1)) != 0) {                                                      \
            uint64_t pow2 = 1;                                                                                                   \
            while (pow2 < new_capacity)                                                                                          \
                pow2 <<= 1;                                                                                                      \
            new_capacity = pow2;                                                                                                 \
        }                                                                                                                        \
        if (new_capacity == 0)                                                                                                   \
            return;                                                                                                              \
                                                                                                                                 \
        void **new_table = (void **)calloc(new_capacity, sizeof(void *));                                                        \
        if (!new_table)                                                                                                          \
            return;                                                                                                              \
                                                                                                                                 \
        uint64_t count = 0;                                                                                                      \
        uint64_t iterate_limit = is_inline ? PHM_INLINE_CAPACITY : old_capacity;                                                 \
        for (uint64_t i = 0; i < iterate_limit; ++i) {                                                                           \
            void *key = old_table_ptr[i];                                                                                        \
            if (key != PHM_EMPTY_SLOT_KEY) {                                                                                     \
                Name##_insert_key(new_table, new_capacity, key);                                                                 \
                count++;                                                                                                         \
                if (count == old_non_null_size)                                                                                  \
                    break;                                                                                                       \
            }                                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        if (!is_inline && old_table_ptr != NULL)                                                                                 \
            free(old_table_ptr);                                                                                                 \
                                                                                                                                 \
        set->table = new_table;                                                                                                  \
        set->current_capacity = new_capacity;                                                                                    \
        set->current_size = old_non_null_size;                                                                                   \
    }                                                                                                                            \
                                                                                                                                 \
    static inline void **Name##_find_slot(Name *set, void *key) {                                                                \
        assert(key != NULL && set->table != set->inline_buffer && set->current_capacity > 0);                                    \
        uint64_t cap_mask = set->current_capacity - 1;                                                                           \
        uint64_t h = Name##_hash_key(key);                                                                                       \
        uint64_t index = h & cap_mask;                                                                                           \
        while (1) {                                                                                                              \
            void **slot_ptr = &set->table[index];                                                                                \
            if (*slot_ptr == PHM_EMPTY_SLOT_KEY || *slot_ptr == key) {                                                           \
                return slot_ptr;                                                                                                 \
            }                                                                                                                    \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE bool Name##_add(Name *set, void *key) {                                                                                \
        if (key == NULL) {                                                                                                       \
            if (set->has_null_element) {                                                                                         \
                return false;                                                                                                    \
            } else {                                                                                                             \
                set->has_null_element = true;                                                                                    \
                return true;                                                                                                     \
            }                                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        if (set->table == set->inline_buffer) {                                                                                  \
            uint64_t first_empty_slot = PHM_INLINE_CAPACITY;                                                                     \
            for (uint64_t i = 0; i < PHM_INLINE_CAPACITY; ++i) {                                                                 \
                if (set->inline_buffer[i] == key) {                                                                              \
                    return false;                                                                                                \
                }                                                                                                                \
                if (set->inline_buffer[i] == PHM_EMPTY_SLOT_KEY && first_empty_slot == PHM_INLINE_CAPACITY) {                    \
                    first_empty_slot = i;                                                                                        \
                }                                                                                                                \
            }                                                                                                                    \
            if (first_empty_slot < PHM_INLINE_CAPACITY) {                                                                        \
                set->inline_buffer[first_empty_slot] = key;                                                                      \
                set->current_size++;                                                                                             \
                return true;                                                                                                     \
            } else {                                                                                                             \
                Name##_grow(set);                                                                                                \
                if (set->table == set->inline_buffer || set->current_capacity == 0) {                                            \
                    return false;                                                                                                \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        bool needs_grow = (set->current_capacity == 0);                                                                          \
        if (!needs_grow && set->current_capacity > 0) {                                                                          \
            needs_grow = (set->current_size + 1) * PHM_LOAD_FACTOR_DEN >= set->current_capacity * PHM_LOAD_FACTOR_NUM;           \
        }                                                                                                                        \
        if (needs_grow) {                                                                                                        \
            Name##_grow(set);                                                                                                    \
            if (set->current_capacity == 0)                                                                                      \
                return false;                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        void **slot_ptr = Name##_find_slot(set, key);                                                                            \
                                                                                                                                 \
        if (*slot_ptr == PHM_EMPTY_SLOT_KEY) {                                                                                   \
            *slot_ptr = key;                                                                                                     \
            set->current_size++;                                                                                                 \
            return true;                                                                                                         \
        } else {                                                                                                                 \
            return false;                                                                                                        \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE bool Name##_contains(Name *set, void *key) {                                                                           \
        if (key == NULL) {                                                                                                       \
            return set->has_null_element;                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        if (set->table == set->inline_buffer) {                                                                                  \
            for (uint64_t i = 0; i < PHM_INLINE_CAPACITY; ++i) {                                                                 \
                if (set->inline_buffer[i] == key) {                                                                              \
                    return true;                                                                                                 \
                }                                                                                                                \
            }                                                                                                                    \
            return false;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        if (set->current_capacity == 0)                                                                                          \
            return false;                                                                                                        \
                                                                                                                                 \
        void **slot_ptr = Name##_find_slot(set, key);                                                                            \
        return (*slot_ptr == key);                                                                                               \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_for_each(Name *set, void (*callback)(void *key, void *user_data), void *user_data) {                       \
        if (set->has_null_element) {                                                                                             \
            callback(NULL, user_data);                                                                                           \
        }                                                                                                                        \
                                                                                                                                 \
        if (set->table == set->inline_buffer) {                                                                                  \
            uint64_t count = 0;                                                                                                  \
            for (uint64_t i = 0; i < PHM_INLINE_CAPACITY; ++i) {                                                                 \
                if (set->inline_buffer[i] != PHM_EMPTY_SLOT_KEY) {                                                               \
                    callback(set->inline_buffer[i], user_data);                                                                  \
                    count++;                                                                                                     \
                    if (count == set->current_size)                                                                              \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        } else if (set->current_capacity > 0) {                                                                                  \
            uint64_t count = 0;                                                                                                  \
            for (uint64_t i = 0; i < set->current_capacity; ++i) {                                                               \
                if (set->table[i] != PHM_EMPTY_SLOT_KEY) {                                                                       \
                    callback(set->table[i], user_data);                                                                          \
                    count++;                                                                                                     \
                    if (count == set->current_size)                                                                              \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_free(Name *set) {                                                                                          \
        if (set->table != set->inline_buffer && set->table != NULL) {                                                            \
            free(set->table);                                                                                                    \
        }                                                                                                                        \
        Name##_init(set);                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_move(Name *dest, Name *source) {                                                                           \
        if (dest == source) {                                                                                                    \
            return;                                                                                                              \
        }                                                                                                                        \
        dest->current_size = source->current_size;                                                                               \
        dest->has_null_element = source->has_null_element;                                                                       \
        if (source->table == source->inline_buffer) {                                                                            \
            memcpy(dest->inline_buffer, source->inline_buffer, sizeof(source->inline_buffer));                                   \
            dest->table = dest->inline_buffer;                                                                                   \
        } else {                                                                                                                 \
            dest->table = source->table;                                                                                         \
            dest->current_capacity = source->current_capacity;                                                                   \
        }                                                                                                                        \
        Name##_init(source);                                                                                                     \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                                                                     \
        Name##_init(dest);                                                                                                       \
        dest->has_null_element = source->has_null_element;                                                                       \
        dest->current_size = source->current_size;                                                                               \
                                                                                                                                 \
        if (source->table == source->inline_buffer) {                                                                            \
            memcpy(dest->inline_buffer, source->inline_buffer, sizeof(source->inline_buffer));                                   \
            dest->table = dest->inline_buffer;                                                                                   \
        } else {                                                                                                                 \
            if (source->current_capacity > 0) {                                                                                  \
                dest->table = (void **)calloc(source->current_capacity, sizeof(void *));                                         \
                if (!dest->table) {                                                                                              \
                    Name##_init(dest);                                                                                           \
                    return;                                                                                                      \
                }                                                                                                                \
                memcpy(dest->table, source->table, source->current_capacity * sizeof(void *));                                   \
                dest->current_capacity = source->current_capacity;                                                               \
            } else {                                                                                                             \
                dest->table = dest->inline_buffer;                                                                               \
            }                                                                                                                    \
        }                                                                                                                        \
    }

#define DEFINE_VECTOR(SCOPE, Type, Name)                                                                                         \
                                                                                                                                 \
    typedef struct {                                                                                                             \
        Type *data;                                                                                                              \
        size_t size;                                                                                                             \
        size_t capacity;                                                                                                         \
        Type inline_data[6];                                                                                                     \
    } Name;                                                                                                                      \
                                                                                                                                 \
    SCOPE void Name##_init(Name *v) {                                                                                            \
        v->data = v->inline_data;                                                                                                \
        v->size = 0;                                                                                                             \
        v->capacity = 6;                                                                                                         \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_initWithSize(Name *v, size_t initialSize) {                                                                \
        if (initialSize <= 6) {                                                                                                  \
            v->data = v->inline_data;                                                                                            \
            v->size = initialSize;                                                                                               \
            v->capacity = 6;                                                                                                     \
            if (initialSize > 0)                                                                                                 \
                memset(&v->inline_data[0], 0, initialSize * sizeof(Type));                                                       \
        } else {                                                                                                                 \
            v->data = (Type *)calloc(initialSize, sizeof(Type));                                                                 \
            if (!v->data && initialSize > 0) {                                                                                   \
                Name##_init(v);                                                                                                  \
                return;                                                                                                          \
            }                                                                                                                    \
            v->size = initialSize;                                                                                               \
            v->capacity = initialSize;                                                                                           \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_free(Name *v) {                                                                                            \
        if (v->data != v->inline_data && v->data != NULL) {                                                                      \
            free(v->data);                                                                                                       \
        }                                                                                                                        \
        v->data = v->inline_data;                                                                                                \
        v->size = 0;                                                                                                             \
        v->capacity = 6;                                                                                                         \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE int Name##_reserve(Name *v, size_t new_capacity) {                                                                     \
        int is_inline = (v->data == v->inline_data);                                                                             \
        if (new_capacity <= v->capacity && !(is_inline && new_capacity > 6)) {                                                   \
            return 0;                                                                                                            \
        }                                                                                                                        \
        if (new_capacity <= 6 && is_inline) {                                                                                    \
            return 0;                                                                                                            \
        }                                                                                                                        \
        Type *new_data;                                                                                                          \
        if (is_inline) {                                                                                                         \
            new_data = (Type *)malloc(new_capacity * sizeof(Type));                                                              \
            if (!new_data && new_capacity > 0)                                                                                   \
                return -1;                                                                                                       \
            if (new_data)                                                                                                        \
                memcpy(new_data, v->inline_data, v->size * sizeof(Type));                                                        \
        } else {                                                                                                                 \
            new_data = (Type *)realloc(v->data, new_capacity * sizeof(Type));                                                    \
            if (!new_data && new_capacity > 0)                                                                                   \
                return -1;                                                                                                       \
        }                                                                                                                        \
        v->data = new_data;                                                                                                      \
        v->capacity = new_capacity;                                                                                              \
        return 0;                                                                                                                \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE int Name##_push_back(Name *v, Type value) {                                                                            \
        if (v->size >= v->capacity) {                                                                                            \
            size_t new_capacity;                                                                                                 \
            if (v->capacity <= 6) {                                                                                              \
                new_capacity = 12;                                                                                               \
            } else {                                                                                                             \
                new_capacity = v->capacity * 2;                                                                                  \
            }                                                                                                                    \
            if (new_capacity < v->capacity)                                                                                      \
                return -1;                                                                                                       \
            if (Name##_reserve(v, new_capacity) != 0)                                                                            \
                return -1;                                                                                                       \
        }                                                                                                                        \
        v->data[v->size++] = value;                                                                                              \
        return 0;                                                                                                                \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE Type *Name##_get(Name *v, size_t index) {                                                                              \
        if (index >= v->size)                                                                                                    \
            return NULL;                                                                                                         \
        return &v->data[index];                                                                                                  \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE size_t Name##_size(const Name *v) { return v->size; }                                                                  \
                                                                                                                                 \
    SCOPE void Name##_move(Name *dest, Name *source) {                                                                           \
        if (dest == source) {                                                                                                    \
            return;                                                                                                              \
        }                                                                                                                        \
        dest->size = source->size;                                                                                               \
        if (source->data == source->inline_data) {                                                                               \
            memcpy(dest->inline_data, source->inline_data, source->size * sizeof(Type));                                         \
            dest->data = dest->inline_data;                                                                                      \
            dest->capacity = 6;                                                                                                  \
        } else {                                                                                                                 \
            dest->data = source->data;                                                                                           \
            dest->capacity = source->capacity;                                                                                   \
        }                                                                                                                        \
        Name##_init(source);                                                                                                     \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                                                                     \
        Name##_init(dest);                                                                                                       \
                                                                                                                                 \
        if (source->size == 0) {                                                                                                 \
            return;                                                                                                              \
        }                                                                                                                        \
                                                                                                                                 \
        if (source->data == source->inline_data) {                                                                               \
            memcpy(dest->inline_data, source->inline_data, source->size * sizeof(Type));                                         \
            dest->data = dest->inline_data;                                                                                      \
            dest->size = source->size;                                                                                           \
            dest->capacity = 6;                                                                                                  \
        } else {                                                                                                                 \
            if (source->capacity > 0) {                                                                                          \
                dest->data = (Type *)malloc(source->capacity * sizeof(Type));                                                    \
                if (!dest->data) {                                                                                               \
                    Name##_init(dest);                                                                                           \
                    return;                                                                                                      \
                }                                                                                                                \
                memcpy(dest->data, source->data, source->size * sizeof(Type));                                                   \
                dest->size = source->size;                                                                                       \
                dest->capacity = source->capacity;                                                                               \
            } else {                                                                                                             \
                /* Should not happen if size > 0, but handle defensively */                                                      \
                Name##_init(dest);                                                                                               \
            }                                                                                                                    \
        }                                                                                                                        \
    }

#endif // HASHMAP_SET_AND_VECTOR_H