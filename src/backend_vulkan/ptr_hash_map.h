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
        Name##_kv_pair* table;                                                  \
        union {                                                                 \
            /* Only valid when table is heap allocated */                       \
            uint64_t current_capacity;                                          \
            /* Storage when inline */                                           \
            Name##_kv_pair inline_buffer[PHM_INLINE_CAPACITY];                  \
        };                                                                      \
    } Name;                                                                     \
                                                                                \
    /* Helper function to hash keys */                                          \
    static inline uint64_t Name##_hash_key(void* key) {                         \
        return (uintptr_t)key * PHM_HASH_MULTIPLIER;                            \
    }                                                                           \
                                                                                \
    /* Forward declaration for grow */                                          \
    static void Name##_grow(Name* map);                                         \
                                                                                \
    /* Initialize a map */                                                      \
    SCOPE void Name##_init(Name* map) {                                         \
        map->current_size = 0;                                                  \
        /* Point table to the inline buffer initially */                        \
        map->table = map->inline_buffer;                                        \
        /* Zero out the inline buffer (clears keys and values) */               \
        memset(map->inline_buffer, 0, sizeof(map->inline_buffer));              \
        /* Capacity field is not used when inline */                            \
    }                                                                           \
                                                                                \
    /* Internal: Insert an entry into a given table (used by grow) */           \
    static void Name##_insert_entry(Name##_kv_pair* table, uint64_t capacity,   \
                                     void* key, ValueType value) {              \
        uint64_t cap_mask = capacity - 1;                                       \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
                                                                                \
        while (1) {                                                             \
            Name##_kv_pair* slot = &table[index];                               \
            /* Use empty slot */                                                \
            if (slot->key == NULL) {                                            \
                slot->key = key;                                                \
                slot->value = value; /* Direct assignment */                    \
                return;                                                         \
            }                                                                   \
            /* Linear probe to next slot */                                     \
            index = (index + 1) & cap_mask;                                     \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Grow the hash map's capacity */                                          \
    static void Name##_grow(Name* map) {                                        \
        bool is_inline = (map->table == map->inline_buffer);                    \
        uint64_t old_size = map->current_size;                                  \
        /* Capacity is fixed when inline, stored in union when heap */          \
        uint64_t old_capacity = is_inline ? PHM_INLINE_CAPACITY : map->current_capacity; \
        Name##_kv_pair* old_table_ptr = map->table;                             \
                                                                                \
        /* Determine new capacity (must be power of 2) */                       \
        uint64_t new_capacity = (old_capacity < PHM_INITIAL_HEAP_CAPACITY)      \
                                  ? PHM_INITIAL_HEAP_CAPACITY                   \
                                  : old_capacity * 2;                           \
        if (new_capacity == 0) new_capacity = PHM_INITIAL_HEAP_CAPACITY;        \
                                                                                \
        /* Allocate and zero the new table */                                   \
        Name##_kv_pair* new_table = (Name##_kv_pair*)calloc(new_capacity, sizeof(Name##_kv_pair)); \
        if (!new_table) {                                                       \
            /* Allocation failure - optional handling: maybe panic or return error */ \
            return;                                                             \
        }                                                                       \
                                                                                \
        /* Re-insert old elements into the new table */                         \
        /* If inline, only iterate up to current_size (packed array assumption)*/\
        /* If heap, iterate through the entire old capacity */                  \
        uint64_t limit = is_inline ? old_size : old_capacity;                   \
        for (uint64_t i = 0; i < limit; ++i) {                                  \
            Name##_kv_pair* entry = &old_table_ptr[i];                          \
            if (entry->key != NULL) {                                           \
                Name##_insert_entry(new_table, new_capacity, entry->key, entry->value); \
            }                                                                   \
        }                                                                       \
                                                                                \
        /* Free the old table if it was heap-allocated */                       \
        if (!is_inline) {                                                       \
            free(old_table_ptr);                                                \
        }                                                                       \
                                                                                \
        /* Update map state */                                                  \
        map->table = new_table;                                                 \
        map->current_capacity = new_capacity;                                   \
        /* Size doesn't change during grow, just capacity/layout */             \
        map->current_size = old_size;                                           \
    }                                                                           \
                                                                                \
    /* Find a slot for a key (used by put and get) */                           \
    /* Returns pointer to the slot, or pointer to empty slot where key *should* go */ \
    /* Returns NULL if map requires growth (only relevant for put) */           \
    static inline Name##_kv_pair* Name##_find_slot(Name* map, void* key) {      \
        uint64_t cap_mask = map->current_capacity - 1;                          \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
                                                                                \
        while (1) {                                                             \
            Name##_kv_pair* slot = &map->table[index];                          \
            /* Found empty slot or matching key */                              \
            if (slot->key == NULL || slot->key == key) {                        \
                return slot;                                                    \
            }                                                                   \
            /* Linear probe */                                                  \
            index = (index + 1) & cap_mask;                                     \
            /* Note: This assumes we never wrap around to the start without */  \
            /* finding an empty slot if the table isn't full. Grow logic */     \
            /* prevents the table from becoming completely full. */             \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Insert or update a key-value pair */                                     \
    SCOPE void Name##_put(Name* map, void* key, ValueType value) {              \
        if (key == NULL) return; /* NULL key not supported */                    \
                                                                                \
        /* --- Inline Storage Case --- */                                       \
        if (map->table == map->inline_buffer) {                                 \
            /* Check if key already exists (linear scan) */                     \
            for (uint64_t i = 0; i < map->current_size; ++i) {                  \
                if (map->inline_buffer[i].key == key) {                         \
                    map->inline_buffer[i].value = value; /* Update */           \
                    return;                                                     \
                }                                                               \
            }                                                                   \
            /* Key not found, check if there's space */                         \
            if (map->current_size < PHM_INLINE_CAPACITY) {                      \
                /* Add new entry */                                             \
                map->inline_buffer[map->current_size].key = key;                \
                map->inline_buffer[map->current_size].value = value;            \
                map->current_size++;                                            \
                return;                                                         \
            } else {                                                            \
                /* Inline buffer full, grow to heap */                          \
                Name##_grow(map);                                               \
                /* Fall through to heap insertion logic */                      \
            }                                                                   \
        }                                                                       \
                                                                                \
        /* --- Heap Storage Case --- */                                         \
        /* Check load factor and grow if needed BEFORE insertion */             \
        /* Ensure capacity is not zero before checking load factor */           \
         if (map->current_capacity > 0 &&                                      \
            map->current_size * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM) { \
            Name##_grow(map);                                                   \
        }                                                                       \
        /* Capacity should be > 0 after potential grow */                       \
        if (map->current_capacity == 0) {                                       \
             /* Should not happen if grow worked, but handle defensively */     \
             /* Or potentially trigger grow here if init state was weird */     \
             Name##_grow(map);                                                  \
             if(map->current_capacity == 0) return; /* Grow failed */           \
        }                                                                       \
                                                                                \
        /* Find the slot for the key */                                         \
        Name##_kv_pair* slot = Name##_find_slot(map, key);                      \
                                                                                \
        /* Insert or update */                                                  \
        if (slot->key == NULL) {                                                \
            /* Insert new key */                                                \
            slot->key = key;                                                    \
            slot->value = value;                                                \
            map->current_size++;                                                \
        } else {                                                                \
            /* Update existing key */                                           \
            slot->value = value;                                                \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Get a pointer to the value associated with a key */                      \
    /* Returns NULL if the key is not found */                                  \
    SCOPE ValueType* Name##_get(Name* map, void* key) {                         \
        if (key == NULL) return NULL; /* NULL key not supported */               \
                                                                                \
        /* --- Inline Storage Case --- */                                       \
        if (map->table == map->inline_buffer) {                                 \
            /* Linear scan */                                                   \
            for (uint64_t i = 0; i < map->current_size; ++i) {                  \
                if (map->inline_buffer[i].key == key) {                         \
                    return &map->inline_buffer[i].value; /* Return address */   \
                }                                                               \
            }                                                                   \
            return NULL; /* Not found */                                        \
        }                                                                       \
                                                                                \
        /* --- Heap Storage Case --- */                                         \
        if (map->current_capacity == 0) return NULL; /* Empty heap table */     \
                                                                                \
        /* Find the slot where the key should be */                             \
        Name##_kv_pair* slot = Name##_find_slot(map, key);                      \
                                                                                \
        /* If key is found return pointer to value, else NULL */                \
        if (slot->key == key) { /* Check key matches (not just non-NULL) */     \
            return &slot->value;                                                \
        } else {                                                                \
            return NULL; /* Not found (find_slot returned an empty slot) */     \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Free the hash map's heap resources (if any) and reset */                 \
    SCOPE void Name##_free(Name* map) {                                         \
        /* Free heap table if it was allocated */                               \
        if (map->table != map->inline_buffer) {                                 \
            free(map->table);                                                   \
        }                                                                       \
        /* Reset to initial inline state */                                     \
        Name##_init(map);                                                       \
    }

#endif // PTR_HASH_MAP_GENERIC_H
#endif