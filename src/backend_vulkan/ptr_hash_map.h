#ifndef PTR_HASH_MAP
#define PTR_HASH_MAP
#ifndef PTR_HASH_MAP_GENERIC_H
#define PTR_HASH_MAP_GENERIC_H

#include <stdint.h>
#include <stdlib.h> // calloc, free
#include <string.h> // memset, memcpy
#include <stdbool.h> // bool type
#include <stddef.h> // offsetof, size_t (though we mostly use uint64_t)

// Configuration Constants (can be overridden before including/defining)
#ifndef PHM_INLINE_CAPACITY
#define PHM_INLINE_CAPACITY 3
#endif

#ifndef PHM_INITIAL_HEAP_CAPACITY
#define PHM_INITIAL_HEAP_CAPACITY 8 // Must be power of 2 >= 1
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

// Macro to generate the hash map implementation
#define DEFINE_PTR_HASH_MAP(SCOPE, Name, ValueType)                             \
                                                                                \
    typedef struct Name##_kv_pair {                                             \
        void* key;                                                              \
        ValueType value;                                                        \
    } Name##_kv_pair;                                                           \
                                                                                \
    typedef struct Name {                                                       \
        uint64_t current_size;                                                  \
        Name##_kv_pair* table; /* Points to inline_buffer or heap */            \
        union {                                                                 \
            uint64_t current_capacity; /* Only valid when table is heap */      \
            Name##_kv_pair inline_buffer[PHM_INLINE_CAPACITY];                  \
        };                                                                      \
    } Name;                                                                     \
                                                                                \
    static inline uint64_t Name##_hash_key(void* key) {                         \
        return (uintptr_t)key * PHM_HASH_MULTIPLIER;                            \
    }                                                                           \
                                                                                \
    static void Name##_grow(Name* map);                                         \
                                                                                \
    SCOPE void Name##_init(Name* map) {                                         \
        map->current_size = 0;                                                  \
        map->table = map->inline_buffer;                                        \
        memset(map->inline_buffer, 0, sizeof(map->inline_buffer));              \
    }                                                                           \
                                                                                \
    static void Name##_insert_entry(Name##_kv_pair* table, uint64_t capacity,   \
                                     void* key, ValueType value) {              \
        uint64_t cap_mask = capacity - 1;                                       \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
        while (1) {                                                             \
            Name##_kv_pair* slot = &table[index];                               \
            if (slot->key == NULL) {                                            \
                slot->key = key;                                                \
                slot->value = value;                                            \
                return;                                                         \
            }                                                                   \
            index = (index + 1) & cap_mask;                                     \
        }                                                                       \
    }                                                                           \
                                                                                \
    static void Name##_grow(Name* map) {                                        \
        bool is_inline = (map->table == map->inline_buffer);                    \
        uint64_t old_size = map->current_size;                                  \
        uint64_t old_capacity = is_inline ? PHM_INLINE_CAPACITY : map->current_capacity; \
        Name##_kv_pair* old_table_ptr = map->table;                             \
                                                                                \
        uint64_t new_capacity = (old_capacity < PHM_INITIAL_HEAP_CAPACITY)      \
                                  ? PHM_INITIAL_HEAP_CAPACITY                   \
                                  : old_capacity * 2;                           \
        if (new_capacity == 0) new_capacity = PHM_INITIAL_HEAP_CAPACITY;        \
                                                                                \
        Name##_kv_pair* new_table = (Name##_kv_pair*)calloc(new_capacity, sizeof(Name##_kv_pair)); \
        if (!new_table) return;                                                 \
                                                                                \
        uint64_t limit = is_inline ? old_size : old_capacity;                   \
        for (uint64_t i = 0; i < limit; ++i) {                                  \
            Name##_kv_pair* entry = &old_table_ptr[i];                          \
            if (entry->key != NULL) {                                           \
                Name##_insert_entry(new_table, new_capacity, entry->key, entry->value); \
            }                                                                   \
        }                                                                       \
                                                                                \
        if (!is_inline) free(old_table_ptr);                                    \
                                                                                \
        map->table = new_table;                                                 \
        map->current_capacity = new_capacity;                                   \
        map->current_size = old_size;                                           \
    }                                                                           \
                                                                                \
    static inline Name##_kv_pair* Name##_find_slot(Name* map, void* key) {      \
        uint64_t cap_mask = map->current_capacity - 1;                          \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
        while (1) {                                                             \
            Name##_kv_pair* slot = &map->table[index];                          \
            if (slot->key == NULL || slot->key == key) {                        \
                return slot;                                                    \
            }                                                                   \
            index = (index + 1) & cap_mask;                                     \
        }                                                                       \
    }                                                                           \
                                                                                \
    SCOPE void Name##_put(Name* map, void* key, ValueType value) {              \
        if (key == NULL) return;                                                \
                                                                                \
        if (map->table == map->inline_buffer) {                                 \
            for (uint64_t i = 0; i < map->current_size; ++i) {                  \
                if (map->inline_buffer[i].key == key) {                         \
                    map->inline_buffer[i].value = value; return;                \
                }                                                               \
            }                                                                   \
            if (map->current_size < PHM_INLINE_CAPACITY) {                      \
                map->inline_buffer[map->current_size].key = key;                \
                map->inline_buffer[map->current_size].value = value;            \
                map->current_size++; return;                                    \
            } else {                                                            \
                Name##_grow(map);                                               \
                if(map->table == map->inline_buffer) return;                    \
            }                                                                   \
        }                                                                       \
                                                                                \
        if (map->current_capacity > 0 &&                                        \
            map->current_size * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM) { \
            Name##_grow(map);                                                   \
            if (map->current_capacity == 0) return;                             \
        }                                                                       \
        if (map->current_capacity == 0) { Name##_grow(map); if (map->current_capacity == 0) return; } \
                                                                                \
        Name##_kv_pair* slot = Name##_find_slot(map, key);                      \
        if (slot->key == NULL) {                                                \
            slot->key = key;                                                    \
            slot->value = value;                                                \
            map->current_size++;                                                \
        } else {                                                                \
            slot->value = value;                                                \
        }                                                                       \
    }                                                                           \
                                                                                \
    SCOPE ValueType* Name##_get(Name* map, void* key) {                         \
        if (key == NULL) return NULL;                                           \
                                                                                \
        if (map->table == map->inline_buffer) {                                 \
            for (uint64_t i = 0; i < map->current_size; ++i) {                  \
                if (map->inline_buffer[i].key == key) {                         \
                    return &map->inline_buffer[i].value;                        \
                }                                                               \
            }                                                                   \
            return NULL;                                                        \
        }                                                                       \
                                                                                \
        if (map->current_capacity == 0) return NULL;                            \
        Name##_kv_pair* slot = Name##_find_slot(map, key);                      \
        return (slot->key == key) ? &slot->value : NULL;                        \
    }                                                                           \
                                                                                \
    SCOPE void Name##_for_each(Name* map, void (*callback)(void* key, ValueType* value, void* user_data), void* user_data) { \
        if (map->table == map->inline_buffer) {                                 \
            for (uint64_t i = 0; i < map->current_size; ++i) {                  \
                callback(map->inline_buffer[i].key, &map->inline_buffer[i].value, user_data); \
            }                                                                   \
        } else if (map->current_capacity > 0) {                                 \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {              \
                if (map->table[i].key != NULL) {                                \
                    callback(map->table[i].key, &map->table[i].value, user_data); \
                }                                                               \
            }                                                                   \
        }                                                                       \
    }                                                                           \
                                                                                \
    SCOPE void Name##_free(Name* map) {                                         \
        if (map->table != map->inline_buffer) {                                 \
            free(map->table);                                                   \
        }                                                                       \
        Name##_init(map);                                                       \
    }

#endif // PTR_HASH_MAP_GENERIC_H
#endif // PTR_HASH_MAP


// =============================================================================
// Pointer Hash Set Implementation
// =============================================================================

#ifndef PTR_HASH_SET
#define PTR_HASH_SET
#ifndef PTR_HASH_SET_GENERIC_H
#define PTR_HASH_SET_GENERIC_H

// Re-include headers needed by set (already included above, but good practice)
#include <stdint.h>
#include <stdlib.h> // calloc, free
#include <string.h> // memset
#include <stdbool.h> // bool type
#include <stddef.h> // size_t

// Configuration Constants are reused from the map definition above

// Macro to generate the hash set implementation
#define DEFINE_PTR_HASH_SET(SCOPE, Name)                                        \
                                                                                \
    typedef struct Name {                                                       \
        uint64_t current_size;                                                  \
        void** table; /* Points to inline_buffer or heap (array of void*) */    \
        union {                                                                 \
            uint64_t current_capacity; /* Only valid when table is heap */      \
            void* inline_buffer[PHM_INLINE_CAPACITY]; /* Stores pointers */     \
        };                                                                      \
    } Name;                                                                     \
                                                                                \
    static inline uint64_t Name##_hash_key(void* key) {                         \
        return (uintptr_t)key * PHM_HASH_MULTIPLIER;                            \
    }                                                                           \
                                                                                \
    static void Name##_grow(Name* set);                                         \
                                                                                \
    SCOPE void Name##_init(Name* set) {                                         \
        set->current_size = 0;                                                  \
        set->table = set->inline_buffer;                                        \
        memset(set->inline_buffer, 0, sizeof(set->inline_buffer));              \
    }                                                                           \
                                                                                \
    static void Name##_insert_key(void** table, uint64_t capacity, void* key) { \
        uint64_t cap_mask = capacity - 1;                                       \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
        while (1) {                                                             \
            void** slot_ptr = &table[index];                                    \
            if (*slot_ptr == NULL) {                                            \
                *slot_ptr = key;                                                \
                return;                                                         \
            }                                                                   \
            index = (index + 1) & cap_mask;                                     \
        }                                                                       \
    }                                                                           \
                                                                                \
    static void Name##_grow(Name* set) {                                        \
        bool is_inline = (set->table == set->inline_buffer);                    \
        uint64_t old_size = set->current_size;                                  \
        uint64_t old_capacity = is_inline ? PHM_INLINE_CAPACITY : set->current_capacity; \
        void** old_table_ptr = set->table;                                      \
                                                                                \
        uint64_t new_capacity = (old_capacity < PHM_INITIAL_HEAP_CAPACITY)      \
                                  ? PHM_INITIAL_HEAP_CAPACITY                   \
                                  : old_capacity * 2;                           \
        if (new_capacity == 0) new_capacity = PHM_INITIAL_HEAP_CAPACITY;        \
                                                                                \
        void** new_table = (void**)calloc(new_capacity, sizeof(void*));         \
        if (!new_table) return;                                                 \
                                                                                \
        uint64_t limit = is_inline ? old_size : old_capacity;                   \
        for (uint64_t i = 0; i < limit; ++i) {                                  \
            void* key = old_table_ptr[i];                                       \
            if (key != NULL) {                                                  \
                Name##_insert_key(new_table, new_capacity, key);                \
            }                                                                   \
        }                                                                       \
                                                                                \
        if (!is_inline) free(old_table_ptr);                                    \
                                                                                \
        set->table = new_table;                                                 \
        set->current_capacity = new_capacity;                                   \
        set->current_size = old_size;                                           \
    }                                                                           \
                                                                                \
    static inline void** Name##_find_slot(Name* set, void* key) {               \
        uint64_t cap_mask = set->current_capacity - 1;                          \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
        while (1) {                                                             \
            void** slot_ptr = &set->table[index];                               \
            if (*slot_ptr == NULL || *slot_ptr == key) {                        \
                return slot_ptr;                                                \
            }                                                                   \
            index = (index + 1) & cap_mask;                                     \
        }                                                                       \
    }                                                                           \
                                                                                \
    SCOPE bool Name##_add(Name* set, void* key) {                               \
        if (key == NULL) return false;                                          \
                                                                                \
        if (set->table == set->inline_buffer) {                                 \
            for (uint64_t i = 0; i < set->current_size; ++i) {                  \
                if (set->inline_buffer[i] == key) {                             \
                    return false;                                               \
                }                                                               \
            }                                                                   \
            if (set->current_size < PHM_INLINE_CAPACITY) {                      \
                set->inline_buffer[set->current_size] = key;                    \
                set->current_size++;                                            \
                return true;                                                    \
            } else {                                                            \
                Name##_grow(set);                                               \
                if(set->table == set->inline_buffer) return false;              \
            }                                                                   \
        }                                                                       \
                                                                                \
        if (set->current_capacity > 0 &&                                        \
            set->current_size * PHM_LOAD_FACTOR_DEN >= set->current_capacity * PHM_LOAD_FACTOR_NUM) { \
            Name##_grow(set);                                                   \
            if (set->current_capacity == 0) return false;                       \
        }                                                                       \
        if (set->current_capacity == 0) { Name##_grow(set); if (set->current_capacity == 0) return false; } \
                                                                                \
        void** slot_ptr = Name##_find_slot(set, key);                           \
        if (*slot_ptr == NULL) {                                                \
            *slot_ptr = key;                                                    \
            set->current_size++;                                                \
            return true;                                                        \
        } else {                                                                \
            return false;                                                       \
        }                                                                       \
    }                                                                           \
                                                                                \
    SCOPE bool Name##_contains(Name* set, void* key) {                          \
        if (key == NULL) return false;                                          \
                                                                                \
        if (set->table == set->inline_buffer) {                                 \
            for (uint64_t i = 0; i < set->current_size; ++i) {                  \
                if (set->inline_buffer[i] == key) {                             \
                    return true;                                                \
                }                                                               \
            }                                                                   \
            return false;                                                       \
        }                                                                       \
                                                                                \
        if (set->current_capacity == 0) return false;                           \
        void** slot_ptr = Name##_find_slot(set, key);                           \
        return (*slot_ptr == key);                                              \
    }                                                                           \
                                                                                \
    SCOPE void Name##_for_each(Name* set, void (*callback)(void* key, void* user_data), void* user_data) { \
        if (set->table == set->inline_buffer) {                                 \
            for (uint64_t i = 0; i < set->current_size; ++i) {                  \
                callback(set->inline_buffer[i], user_data);                     \
            }                                                                   \
        } else if (set->current_capacity > 0) {                                 \
            for (uint64_t i = 0; i < set->current_capacity; ++i) {              \
                if (set->table[i] != NULL) {                                    \
                    callback(set->table[i], user_data);                         \
                }                                                               \
            }                                                                   \
        }                                                                       \
    }                                                                           \
                                                                                \
    SCOPE void Name##_free(Name* set) {                                         \
        if (set->table != set->inline_buffer) {                                 \
            free(set->table);                                                   \
        }                                                                       \
        Name##_init(set);                                                       \
    }

#endif // PTR_HASH_SET_GENERIC_H
#endif // PTR_HASH_SET