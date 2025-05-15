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
        uint64_t current_size;     /* Number of non-NULL keys */                                                                 \
        uint64_t current_capacity; /* Capacity of the heap-allocated table */                                                    \
        bool has_null_key;                                                                                                       \
        ValueType null_value;                                                                                                    \
        Name##_kv_pair *table; /* Pointer to the hash table data (heap-allocated) */                                             \
    } Name;                                                                                                                      \
                                                                                                                                 \
    static inline uint64_t Name##_hash_key(void *key) {                                                                          \
        assert(key != NULL);                                                                                                     \
        return ((uintptr_t)key) * PHM_HASH_MULTIPLIER;                                                                           \
    }                                                                                                                            \
                                                                                                                                 \
    /* Helper to round up to the next power of 2. Result can be 0 if v is 0 or on overflow from UINT64_MAX. */                   \
    static inline uint64_t Name##_round_up_to_power_of_2(uint64_t v) {                                                           \
        if (v == 0)                                                                                                              \
            return 0;                                                                                                            \
        v--;                                                                                                                     \
        v |= v >> 1;                                                                                                             \
        v |= v >> 2;                                                                                                             \
        v |= v >> 4;                                                                                                             \
        v |= v >> 8;                                                                                                             \
        v |= v >> 16;                                                                                                            \
        v |= v >> 32;                                                                                                            \
        v++;                                                                                                                     \
        return v;                                                                                                                \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_insert_entry(Name##_kv_pair *table, uint64_t capacity, void *key, ValueType value) {                      \
        assert(key != NULL && key != PHM_EMPTY_SLOT_KEY && capacity > 0 && (capacity & (capacity - 1)) == 0);                    \
        uint64_t cap_mask = capacity - 1;                                                                                        \
        uint64_t index = Name##_hash_key(key) & cap_mask;                                                                        \
        while (table[index].key != PHM_EMPTY_SLOT_KEY) {                                                                         \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
        table[index].key = key;                                                                                                  \
        table[index].value = value;                                                                                              \
    }                                                                                                                            \
                                                                                                                                 \
    static Name##_kv_pair *Name##_find_slot(Name *map, void *key) {                                                              \
        assert(key != NULL && key != PHM_EMPTY_SLOT_KEY && map->table != NULL && map->current_capacity > 0);                     \
        uint64_t cap_mask = map->current_capacity - 1;                                                                           \
        uint64_t index = Name##_hash_key(key) & cap_mask;                                                                        \
        while (map->table[index].key != PHM_EMPTY_SLOT_KEY && map->table[index].key != key) {                                    \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
        return &map->table[index];                                                                                               \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_grow(Name *map); /* Forward declaration */                                                                \
                                                                                                                                 \
    SCOPE void Name##_init(Name *map) {                                                                                          \
        map->current_size = 0;                                                                                                   \
        map->current_capacity = 0;                                                                                               \
        map->has_null_key = false;                                                                                               \
        /* map->null_value is uninitialized, which is fine */                                                                    \
        map->table = NULL;                                                                                                       \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_grow(Name *map) {                                                                                         \
        uint64_t old_capacity = map->current_capacity;                                                                           \
        Name##_kv_pair *old_table = map->table;                                                                                  \
        uint64_t new_capacity;                                                                                                   \
                                                                                                                                 \
        if (old_capacity == 0) {                                                                                                 \
            new_capacity = (PHM_INITIAL_HEAP_CAPACITY > 0) ? PHM_INITIAL_HEAP_CAPACITY : 8; /* Default 8 if initial is 0 */      \
        } else {                                                                                                                 \
            if (old_capacity >= (UINT64_MAX / 2))                                                                                \
                new_capacity = UINT64_MAX; /* Avoid overflow */                                                                  \
            else                                                                                                                 \
                new_capacity = old_capacity * 2;                                                                                 \
        }                                                                                                                        \
                                                                                                                                 \
        new_capacity = Name##_round_up_to_power_of_2(new_capacity);                                                              \
        if (new_capacity == 0 && old_capacity == 0 && ((PHM_INITIAL_HEAP_CAPACITY > 0) ? PHM_INITIAL_HEAP_CAPACITY : 8) > 0) {   \
            /* This case means round_up_to_power_of_2 resulted in 0 from a non-zero initial desired capacity (e.g. UINT64_MAX)   \
             */                                                                                                                  \
            /* If PHM_INITIAL_HEAP_CAPACITY was huge and overflowed round_up. Use max power of 2. */                             \
            new_capacity = (UINT64_C(1) << 63);                                                                                  \
        }                                                                                                                        \
                                                                                                                                 \
        if (new_capacity == 0 || (new_capacity <= old_capacity && old_capacity > 0)) {                                           \
            return; /* Cannot grow or no actual increase in capacity */                                                          \
        }                                                                                                                        \
                                                                                                                                 \
        Name##_kv_pair *new_table = (Name##_kv_pair *)calloc(new_capacity, sizeof(Name##_kv_pair));                              \
        if (!new_table)                                                                                                          \
            return; /* Allocation failure */                                                                                     \
                                                                                                                                 \
        if (old_table && map->current_size > 0) {                                                                                \
            uint64_t rehashed_count = 0;                                                                                         \
            for (uint64_t i = 0; i < old_capacity; ++i) {                                                                        \
                if (old_table[i].key != PHM_EMPTY_SLOT_KEY) {                                                                    \
                    Name##_insert_entry(new_table, new_capacity, old_table[i].key, old_table[i].value);                          \
                    rehashed_count++;                                                                                            \
                    if (rehashed_count == map->current_size)                                                                     \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
        if (old_table)                                                                                                           \
            free(old_table);                                                                                                     \
        map->table = new_table;                                                                                                  \
        map->current_capacity = new_capacity;                                                                                    \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE int Name##_put(Name *map, void *key, ValueType value) {                                                                \
        if (key == NULL) {                                                                                                       \
            map->null_value = value;                                                                                             \
            if (!map->has_null_key) {                                                                                            \
                map->has_null_key = true;                                                                                        \
                return 1; /* New NULL key */                                                                                     \
            }                                                                                                                    \
            return 0; /* Updated NULL key */                                                                                     \
        }                                                                                                                        \
        assert(key != PHM_EMPTY_SLOT_KEY);                                                                                       \
                                                                                                                                 \
        if (map->current_capacity == 0 ||                                                                                        \
            (map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM) {                      \
            uint64_t old_cap = map->current_capacity;                                                                            \
            Name##_grow(map);                                                                                                    \
            if (map->current_capacity == old_cap && old_cap > 0) { /* Grow failed or no increase */                              \
                /* Re-check if still insufficient */                                                                             \
                if ((map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM)                \
                    return 0;                                                                                                    \
            } else if (map->current_capacity == 0)                                                                               \
                return 0; /* Grow failed to allocate any capacity */                                                             \
            else if ((map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM)               \
                return 0;                                                                                                        \
        }                                                                                                                        \
        assert(map->current_capacity > 0 && map->table != NULL); /* Must have capacity after grow check */                       \
                                                                                                                                 \
        Name##_kv_pair *slot = Name##_find_slot(map, key);                                                                       \
        if (slot->key == PHM_EMPTY_SLOT_KEY) {                                                                                   \
            slot->key = key;                                                                                                     \
            slot->value = value;                                                                                                 \
            map->current_size++;                                                                                                 \
            return 1; /* New key */                                                                                              \
        } else {                                                                                                                 \
            assert(slot->key == key);                                                                                            \
            slot->value = value;                                                                                                 \
            return 0; /* Updated existing key */                                                                                 \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE ValueType *Name##_get(Name *map, void *key) {                                                                          \
        if (key == NULL)                                                                                                         \
            return map->has_null_key ? &map->null_value : NULL;                                                                  \
        assert(key != PHM_EMPTY_SLOT_KEY);                                                                                       \
        if (map->current_capacity == 0 || map->table == NULL)                                                                    \
            return NULL;                                                                                                         \
        Name##_kv_pair *slot = Name##_find_slot(map, key);                                                                       \
        return (slot->key == key) ? &slot->value : NULL;                                                                         \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_for_each(Name *map, void (*callback)(void *key, ValueType *value, void *user_data), void *user_data) {     \
        if (map->has_null_key)                                                                                                   \
            callback(NULL, &map->null_value, user_data);                                                                         \
        if (map->current_capacity > 0 && map->table != NULL && map->current_size > 0) {                                          \
            uint64_t count = 0;                                                                                                  \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                               \
                if (map->table[i].key != PHM_EMPTY_SLOT_KEY) {                                                                   \
                    callback(map->table[i].key, &map->table[i].value, user_data);                                                \
                    if (++count == map->current_size)                                                                            \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_free(Name *map) {                                                                                          \
        if (map->table != NULL)                                                                                                  \
            free(map->table);                                                                                                    \
        Name##_init(map); /* Reset to initial empty state */                                                                     \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_move(Name *dest, Name *source) {                                                                           \
        if (dest == source)                                                                                                      \
            return;                                                                                                              \
        if (dest->table != NULL)                                                                                                 \
            free(dest->table); /* Free existing dest resources */                                                                \
        *dest = *source;       /* Copy all members, dest now owns source's table */                                              \
        Name##_init(source);   /* Reset source to prevent double free */                                                         \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_clear(Name *map) {                                                                                         \
        map->current_size = 0;                                                                                                   \
        map->has_null_key = false;                                                                                               \
        if (map->table != NULL && map->current_capacity > 0) {                                                                   \
            /* calloc already zeroed memory if PHM_EMPTY_SLOT_KEY is 0. */                                                       \
            /* If PHM_EMPTY_SLOT_KEY is not 0, or for robustness: */                                                             \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                               \
                map->table[i].key = PHM_EMPTY_SLOT_KEY;                                                                          \
                /* map->table[i].value = (ValueType){0}; // Optional: if values need resetting */                                \
            }                                                                                                                    \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                                                                     \
        if (dest == source)                                                                                                      \
            return;                                                                                                              \
        if (dest->table != NULL)                                                                                                 \
            free(dest->table);                                                                                                   \
        Name##_init(dest); /* Initialize dest to a clean empty state */                                                          \
                                                                                                                                 \
        dest->has_null_key = source->has_null_key;                                                                               \
        if (source->has_null_key)                                                                                                \
            dest->null_value = source->null_value;                                                                               \
        dest->current_size = source->current_size;                                                                               \
                                                                                                                                 \
        if (source->table != NULL && source->current_capacity > 0) {                                                             \
            dest->table = (Name##_kv_pair *)calloc(source->current_capacity, sizeof(Name##_kv_pair));                            \
            if (!dest->table) {                                                                                                  \
                Name##_init(dest);                                                                                               \
                return;                                                                                                          \
            } /* Alloc fail, reset dest to safe empty */                                                                         \
            memcpy(dest->table, source->table, source->current_capacity * sizeof(Name##_kv_pair));                               \
            dest->current_capacity = source->current_capacity;                                                                   \
        }                                                                                                                        \
        /* If source had no table, dest remains in its _init state (table=NULL, capacity=0) */                                   \
    }

#define DEFINE_PTR_HASH_SET(SCOPE, Name, Type)                                                                                   \
                                                                                                                                 \
    typedef struct Name {                                                                                                        \
        uint64_t current_size;                                                                                                   \
        bool has_null_element;                                                                                                   \
        Type *table;                                                                                                             \
        uint64_t current_capacity;                                                                                               \
    } Name;                                                                                                                      \
                                                                                                                                 \
    static inline uint64_t Name##_hash_key(Type key) { return (uint64_t)key * PHM_HASH_MULTIPLIER; }                             \
                                                                                                                                 \
    static void Name##_grow(Name *set);                                                                                          \
                                                                                                                                 \
    SCOPE void Name##_init(Name *set) {                                                                                          \
        set->current_size = 0;                                                                                                   \
        set->has_null_element = false;                                                                                           \
        set->table = NULL;                                                                                                       \
        set->current_capacity = 0;                                                                                               \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_insert_key(Type *table, uint64_t capacity, Type key) {                                                    \
        assert(key != (Type)PHM_EMPTY_SLOT_KEY);                                                                                 \
        assert(capacity > 0 && (capacity & (capacity - 1)) == 0);                                                                \
        uint64_t cap_mask = capacity - 1;                                                                                        \
        uint64_t h = Name##_hash_key(key);                                                                                       \
        uint64_t index = h & cap_mask;                                                                                           \
        while (1) {                                                                                                              \
            Type *slot_ptr = &table[index];                                                                                      \
            if (*slot_ptr == (Type)PHM_EMPTY_SLOT_KEY) {                                                                         \
                *slot_ptr = key;                                                                                                 \
                return;                                                                                                          \
            }                                                                                                                    \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    static void Name##_grow(Name *set) {                                                                                         \
        uint64_t old_non_null_size = set->current_size;                                                                          \
        uint64_t old_capacity = set->current_capacity;                                                                           \
        Type *old_table_ptr = set->table;                                                                                        \
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
        Type *new_table = (Type *)calloc(new_capacity, sizeof(Type));                                                            \
        if (!new_table)                                                                                                          \
            return;                                                                                                              \
                                                                                                                                 \
        uint64_t count = 0;                                                                                                      \
        for (uint64_t i = 0; i < old_capacity; ++i) {                                                                            \
            Type key = old_table_ptr[i];                                                                                         \
            if (key != (Type)PHM_EMPTY_SLOT_KEY) {                                                                               \
                Name##_insert_key(new_table, new_capacity, key);                                                                 \
                count++;                                                                                                         \
                if (count == old_non_null_size)                                                                                  \
                    break;                                                                                                       \
            }                                                                                                                    \
        }                                                                                                                        \
                                                                                                                                 \
        if (old_table_ptr != NULL)                                                                                               \
            free(old_table_ptr);                                                                                                 \
                                                                                                                                 \
        set->table = new_table;                                                                                                  \
        set->current_capacity = new_capacity;                                                                                    \
        set->current_size = old_non_null_size;                                                                                   \
    }                                                                                                                            \
                                                                                                                                 \
    static inline Type *Name##_find_slot(Name *set, Type key) {                                                                  \
        assert(key != (Type)PHM_EMPTY_SLOT_KEY && set->table != NULL && set->current_capacity > 0);                              \
        uint64_t cap_mask = set->current_capacity - 1;                                                                           \
        uint64_t h = Name##_hash_key(key);                                                                                       \
        uint64_t index = h & cap_mask;                                                                                           \
        while (1) {                                                                                                              \
            Type *slot_ptr = &set->table[index];                                                                                 \
            if (*slot_ptr == (Type)PHM_EMPTY_SLOT_KEY || *slot_ptr == key) {                                                     \
                return slot_ptr;                                                                                                 \
            }                                                                                                                    \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE bool Name##_add(Name *set, Type key) {                                                                                 \
        if (key == (Type)0) {                                                                                                    \
            if (set->has_null_element) {                                                                                         \
                return false;                                                                                                    \
            } else {                                                                                                             \
                set->has_null_element = true;                                                                                    \
                return true;                                                                                                     \
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
        Type *slot_ptr = Name##_find_slot(set, key);                                                                             \
                                                                                                                                 \
        if (*slot_ptr == (Type)PHM_EMPTY_SLOT_KEY) {                                                                             \
            *slot_ptr = key;                                                                                                     \
            set->current_size++;                                                                                                 \
            return true;                                                                                                         \
        } else {                                                                                                                 \
            return false;                                                                                                        \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE bool Name##_contains(Name *set, Type key) {                                                                            \
        if (key == (Type)0) {                                                                                                    \
            return set->has_null_element;                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        if (set->current_capacity == 0)                                                                                          \
            return false;                                                                                                        \
                                                                                                                                 \
        Type *slot_ptr = Name##_find_slot(set, key);                                                                             \
        return (*slot_ptr == key);                                                                                               \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_for_each(Name *set, void (*callback)(Type key, void *user_data), void *user_data) {                        \
        if (set->has_null_element) {                                                                                             \
            callback((Type)0, user_data);                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        if (set->table != NULL && set->current_capacity > 0) {                                                                   \
            uint64_t count = 0;                                                                                                  \
            for (uint64_t i = 0; i < set->current_capacity; ++i) {                                                               \
                if (set->table[i] != (Type)PHM_EMPTY_SLOT_KEY) {                                                                 \
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
        if (set->table != NULL) {                                                                                                \
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
        dest->table = source->table;                                                                                             \
        dest->current_capacity = source->current_capacity;                                                                       \
        Name##_init(source);                                                                                                     \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                                                                     \
        Name##_init(dest);                                                                                                       \
        dest->has_null_element = source->has_null_element;                                                                       \
        dest->current_size = source->current_size;                                                                               \
                                                                                                                                 \
        if (source->table != NULL && source->current_capacity > 0) {                                                             \
            dest->table = (Type *)calloc(source->current_capacity, sizeof(Type));                                                \
            if (!dest->table) {                                                                                                  \
                Name##_init(dest);                                                                                               \
                return;                                                                                                          \
            }                                                                                                                    \
            memcpy(dest->table, source->table, source->current_capacity * sizeof(Type));                                         \
            dest->current_capacity = source->current_capacity;                                                                   \
        }                                                                                                                        \
    }

#define DEFINE_VECTOR(SCOPE, Type, Name)                                                                                         \
                                                                                                                                 \
    typedef struct {                                                                                                             \
        Type *data;                                                                                                              \
        size_t size;                                                                                                             \
        size_t capacity;                                                                                                         \
    } Name;                                                                                                                      \
                                                                                                                                 \
    SCOPE void Name##_init(Name *v) {                                                                                            \
        v->data = NULL;                                                                                                          \
        v->size = 0;                                                                                                             \
        v->capacity = 0;                                                                                                         \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE bool Name##_empty(Name *v) { return v->size == 0; }                                                                    \
                                                                                                                                 \
    SCOPE void Name##_initWithSize(Name *v, size_t initialSize) {                                                                \
        if (initialSize == 0) {                                                                                                  \
            Name##_init(v);                                                                                                      \
        } else {                                                                                                                 \
            v->data = (Type *)calloc(initialSize, sizeof(Type));                                                                 \
            if (!v->data) {                                                                                                      \
                Name##_init(v);                                                                                                  \
                return;                                                                                                          \
            }                                                                                                                    \
            v->size = initialSize;                                                                                               \
            v->capacity = initialSize;                                                                                           \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_free(Name *v) {                                                                                            \
        if (v->data != NULL) {                                                                                                   \
            free(v->data);                                                                                                       \
        }                                                                                                                        \
        Name##_init(v);                                                                                                          \
    }                                                                                                                            \
    SCOPE void Name##_clear(Name *v) { v->size = 0; }                                                                            \
    SCOPE int Name##_reserve(Name *v, size_t new_capacity) {                                                                     \
        if (new_capacity <= v->capacity) {                                                                                       \
            return 0;                                                                                                            \
        }                                                                                                                        \
        Type *new_data = (Type *)realloc(v->data, new_capacity * sizeof(Type));                                                  \
        if (!new_data && new_capacity > 0) {                                                                                     \
            return -1;                                                                                                           \
        }                                                                                                                        \
        v->data = new_data;                                                                                                      \
        v->capacity = new_capacity;                                                                                              \
        return 0;                                                                                                                \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE int Name##_push_back(Name *v, Type value) {                                                                            \
        if (v->size >= v->capacity) {                                                                                            \
            size_t new_capacity;                                                                                                 \
            if (v->capacity == 0) {                                                                                              \
                new_capacity = 8;                                                                                                \
            } else {                                                                                                             \
                new_capacity = v->capacity * 2;                                                                                  \
            }                                                                                                                    \
            if (new_capacity < v->capacity) {                                                                                    \
                return -1;                                                                                                       \
            }                                                                                                                    \
            if (Name##_reserve(v, new_capacity) != 0) {                                                                          \
                return -1;                                                                                                       \
            }                                                                                                                    \
        }                                                                                                                        \
        v->data[v->size++] = value;                                                                                              \
        return 0;                                                                                                                \
    }                                                                                                                            \
    SCOPE void Name##_pop_back(Name *v) { --v->size; }                                                                           \
    SCOPE Type *Name##_get(Name *v, size_t index) {                                                                              \
        if (index >= v->size) {                                                                                                  \
            return NULL;                                                                                                         \
        }                                                                                                                        \
        return &v->data[index];                                                                                                  \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE size_t Name##_size(const Name *v) { return v->size; }                                                                  \
                                                                                                                                 \
    SCOPE void Name##_move(Name *dest, Name *source) {                                                                           \
        if (dest == source) {                                                                                                    \
            return;                                                                                                              \
        }                                                                                                                        \
        dest->data = source->data;                                                                                               \
        dest->size = source->size;                                                                                               \
        dest->capacity = source->capacity;                                                                                       \
        Name##_init(source);                                                                                                     \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                                                                     \
        Name##_free(dest);                                                                                                       \
                                                                                                                                 \
        if (source->size == 0) {                                                                                                 \
            return;                                                                                                              \
        }                                                                                                                        \
                                                                                                                                 \
        size_t capacity_to_allocate = source->capacity;                                                                          \
        if (capacity_to_allocate < source->size) {                                                                               \
            capacity_to_allocate = source->size;                                                                                 \
        }                                                                                                                        \
                                                                                                                                 \
        dest->data = (Type *)malloc(capacity_to_allocate * sizeof(Type));                                                        \
        if (!dest->data) {                                                                                                       \
            return;                                                                                                              \
        }                                                                                                                        \
        memcpy(dest->data, source->data, source->size * sizeof(Type));                                                           \
        dest->size = source->size;                                                                                               \
        dest->capacity = capacity_to_allocate;                                                                                   \
    }
#define DEFINE_GENERIC_HASH_MAP(SCOPE, Name, KeyType, ValueType, KeyHashFunc, KeyCmpFunc, KeyEmptyVal)                           \
    typedef struct Name##_kv_pair {                                                                                              \
        KeyType key;                                                                                                             \
        ValueType value;                                                                                                         \
    } Name##_kv_pair;                                                                                                            \
    typedef struct Name {                                                                                                        \
        uint64_t current_size;                                                                                                   \
        uint64_t current_capacity;                                                                                               \
        KeyType empty_key_sentinel;                                                                                              \
        Name##_kv_pair *table;                                                                                                   \
    } Name;                                                                                                                      \
    static inline uint64_t Name##_hash_key_internal(KeyType key) { return KeyHashFunc(key); }                                    \
    static inline uint64_t Name##_round_up_to_power_of_2(uint64_t v) {                                                           \
        if (v == 0)                                                                                                              \
            return 0;                                                                                                            \
        v--;                                                                                                                     \
        v |= v >> 1;                                                                                                             \
        v |= v >> 2;                                                                                                             \
        v |= v >> 4;                                                                                                             \
        v |= v >> 8;                                                                                                             \
        v |= v >> 16;                                                                                                            \
        v |= v >> 32;                                                                                                            \
        v++;                                                                                                                     \
        return v;                                                                                                                \
    }                                                                                                                            \
    static void Name##_insert_entry(                                                                                             \
        Name##_kv_pair *table, uint64_t capacity, KeyType key, ValueType value, KeyType empty_key_val_param) {                   \
        assert(!KeyCmpFunc(key, empty_key_val_param) && capacity > 0 && (capacity & (capacity - 1)) == 0);                       \
        uint64_t cap_mask = capacity - 1;                                                                                        \
        uint64_t index = Name##_hash_key_internal(key) & cap_mask;                                                               \
        while (!KeyCmpFunc(table[index].key, empty_key_val_param)) {                                                             \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
        table[index].key = key;                                                                                                  \
        table[index].value = value;                                                                                              \
    }                                                                                                                            \
    static Name##_kv_pair *Name##_find_slot(Name *map, KeyType key) {                                                            \
        assert(map->table != NULL && map->current_capacity > 0);                                                                 \
        uint64_t cap_mask = map->current_capacity - 1;                                                                           \
        uint64_t index = Name##_hash_key_internal(key) & cap_mask;                                                               \
        while (!KeyCmpFunc(map->table[index].key, map->empty_key_sentinel) && !KeyCmpFunc(map->table[index].key, key)) {         \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
        return &map->table[index];                                                                                               \
    }                                                                                                                            \
    static void Name##_grow(Name *map);                                                                                          \
    SCOPE void Name##_init(Name *map) {                                                                                          \
        map->current_size = 0;                                                                                                   \
        map->current_capacity = 0;                                                                                               \
        map->empty_key_sentinel = (KeyEmptyVal);                                                                                 \
        map->table = NULL;                                                                                                       \
    }                                                                                                                            \
    static void Name##_grow(Name *map) {                                                                                         \
        uint64_t old_capacity = map->current_capacity;                                                                           \
        Name##_kv_pair *old_table = map->table;                                                                                  \
        uint64_t new_capacity;                                                                                                   \
        if (old_capacity == 0) {                                                                                                 \
            new_capacity = (PHM_INITIAL_HEAP_CAPACITY > 0) ? PHM_INITIAL_HEAP_CAPACITY : 8;                                      \
        } else {                                                                                                                 \
            if (old_capacity >= (UINT64_MAX / 2))                                                                                \
                new_capacity = UINT64_MAX;                                                                                       \
            else                                                                                                                 \
                new_capacity = old_capacity * 2;                                                                                 \
        }                                                                                                                        \
        new_capacity = Name##_round_up_to_power_of_2(new_capacity);                                                              \
        if (new_capacity == 0 && old_capacity == 0 && ((PHM_INITIAL_HEAP_CAPACITY > 0) ? PHM_INITIAL_HEAP_CAPACITY : 8) > 0) {   \
            new_capacity = (UINT64_C(1) << 63);                                                                                  \
        }                                                                                                                        \
        if (new_capacity == 0 || (new_capacity <= old_capacity && old_capacity > 0)) {                                           \
            return;                                                                                                              \
        }                                                                                                                        \
        Name##_kv_pair *new_table = (Name##_kv_pair *)malloc(new_capacity * sizeof(Name##_kv_pair));                             \
        if (!new_table)                                                                                                          \
            return;                                                                                                              \
        for (uint64_t i = 0; i < new_capacity; ++i) {                                                                            \
            new_table[i].key = map->empty_key_sentinel;                                                                          \
        }                                                                                                                        \
        if (old_table && map->current_size > 0) {                                                                                \
            uint64_t rehashed_count = 0;                                                                                         \
            for (uint64_t i = 0; i < old_capacity; ++i) {                                                                        \
                if (!KeyCmpFunc(old_table[i].key, map->empty_key_sentinel)) {                                                    \
                    Name##_insert_entry(new_table, new_capacity, old_table[i].key, old_table[i].value, map->empty_key_sentinel); \
                    rehashed_count++;                                                                                            \
                    if (rehashed_count == map->current_size)                                                                     \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
        if (old_table)                                                                                                           \
            free(old_table);                                                                                                     \
        map->table = new_table;                                                                                                  \
        map->current_capacity = new_capacity;                                                                                    \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE int Name##_put(Name *map, KeyType key, ValueType value) {                                                              \
        assert(!KeyCmpFunc(key, map->empty_key_sentinel));                                                                       \
        if (map->current_capacity == 0 ||                                                                                        \
            (map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM) {                      \
            uint64_t old_cap = map->current_capacity;                                                                            \
            Name##_grow(map);                                                                                                    \
            if (map->current_capacity == old_cap && old_cap > 0) {                                                               \
                if ((map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM)                \
                    return 0;                                                                                                    \
            } else if (map->current_capacity == 0)                                                                               \
                return 0;                                                                                                        \
            else if ((map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM)               \
                return 0;                                                                                                        \
        }                                                                                                                        \
        assert(map->current_capacity > 0 && map->table != NULL);                                                                 \
        Name##_kv_pair *slot = Name##_find_slot(map, key);                                                                       \
        if (KeyCmpFunc(slot->key, map->empty_key_sentinel)) {                                                                    \
            slot->key = key;                                                                                                     \
            slot->value = value;                                                                                                 \
            map->current_size++;                                                                                                 \
            return 1;                                                                                                            \
        } else {                                                                                                                 \
            assert(KeyCmpFunc(slot->key, key));                                                                                  \
            slot->value = value;                                                                                                 \
            return 0;                                                                                                            \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE ValueType *Name##_get(Name *map, KeyType key) {                                                                        \
        assert(!KeyCmpFunc(key, map->empty_key_sentinel));                                                                       \
        if (map->current_capacity == 0 || map->table == NULL)                                                                    \
            return NULL;                                                                                                         \
        Name##_kv_pair *slot = Name##_find_slot(map, key);                                                                       \
        return (KeyCmpFunc(slot->key, key)) ? &slot->value : NULL;                                                               \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_for_each(Name *map, void (*callback)(KeyType key, ValueType * value, void *user_data), void *user_data) {  \
        if (map->current_capacity > 0 && map->table != NULL && map->current_size > 0) {                                          \
            uint64_t count = 0;                                                                                                  \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                               \
                if (!KeyCmpFunc(map->table[i].key, map->empty_key_sentinel)) {                                                   \
                    callback(map->table[i].key, &map->table[i].value, user_data);                                                \
                    if (++count == map->current_size)                                                                            \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_free(Name *map) {                                                                                          \
        if (map->table != NULL)                                                                                                  \
            free(map->table);                                                                                                    \
        Name##_init(map);                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_move(Name *dest, Name *source) {                                                                           \
        if (dest == source)                                                                                                      \
            return;                                                                                                              \
        if (dest->table != NULL)                                                                                                 \
            free(dest->table);                                                                                                   \
        *dest = *source;                                                                                                         \
        Name##_init(source);                                                                                                     \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_clear(Name *map) {                                                                                         \
        map->current_size = 0;                                                                                                   \
        if (map->table != NULL && map->current_capacity > 0) {                                                                   \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                               \
                map->table[i].key = map->empty_key_sentinel;                                                                     \
            }                                                                                                                    \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                                                                     \
        if (dest == source)                                                                                                      \
            return;                                                                                                              \
        if (dest->table != NULL)                                                                                                 \
            free(dest->table);                                                                                                   \
        Name##_init(dest);                                                                                                       \
        dest->current_size = source->current_size;                                                                               \
        if (source->table != NULL && source->current_capacity > 0) {                                                             \
            dest->table = (Name##_kv_pair *)malloc(source->current_capacity * sizeof(Name##_kv_pair));                           \
            if (!dest->table) {                                                                                                  \
                Name##_init(dest);                                                                                               \
                return;                                                                                                          \
            }                                                                                                                    \
            memcpy(dest->table, source->table, source->current_capacity * sizeof(Name##_kv_pair));                               \
            dest->current_capacity = source->current_capacity;                                                                   \
        }                                                                                                                        \
    }

#endif // HASHMAP_SET_AND_VECTOR_H