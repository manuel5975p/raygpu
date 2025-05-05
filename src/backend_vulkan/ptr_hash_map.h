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
// Fibonacci hashing prime multiplier for 64-bit
// 0x9E3779B97F4A7C15ULL is floor(2^64 / phi) where phi is the golden ratio
#define PHM_HASH_MULTIPLIER 0x9E3779B97F4A7C15ULL
#endif

// Sentinel value for empty slots in the hash table (distinct from NULL key)
// We will continue using NULL pointer in the key field to signify empty/deleted
// but handle the actual NULL key separately.
#define PHM_EMPTY_SLOT_KEY NULL
// We need a separate sentinel for deleted slots if we implement removal.
// For now, we only need to distinguish occupied vs empty.

// Macro to generate the hash map implementation
#define DEFINE_PTR_HASH_MAP(SCOPE, Name, ValueType)                             \
                                                                                \
    typedef struct Name##_kv_pair {                                             \
        void* key; /* PHM_EMPTY_SLOT_KEY indicates empty slot */                \
        ValueType value;                                                        \
    } Name##_kv_pair;                                                           \
                                                                                \
    typedef struct Name {                                                       \
        uint64_t current_size; /* Number of *non-NULL* keys */                  \
        bool has_null_key;     /* Flag indicating if NULL key is present */      \
        ValueType null_value;  /* Value associated with NULL key */            \
        Name##_kv_pair* table; /* Points to inline_buffer or heap */            \
        union {                                                                 \
            uint64_t current_capacity; /* Only valid when table is heap */      \
            Name##_kv_pair inline_buffer[PHM_INLINE_CAPACITY];                  \
        };                                                                      \
    } Name;                                                                     \
                                                                                \
    /* Hash function remains the same, only used for non-NULL keys */           \
    static inline uint64_t Name##_hash_key(void* key) {                         \
        /* Assert(key != NULL) could be added here */                           \
        return (uintptr_t)key * PHM_HASH_MULTIPLIER;                            \
    }                                                                           \
                                                                                \
    static void Name##_grow(Name* map);                                         \
                                                                                \
    /* Initializes the map, including NULL key state */                         \
    SCOPE void Name##_init(Name* map) {                                         \
        map->current_size = 0;                                                  \
        map->has_null_key = false;                                              \
        /* Optional: Zero out null_value if ValueType isn't complex */          \
        /* memset(&map->null_value, 0, sizeof(ValueType)); */                  \
        map->table = map->inline_buffer;                                        \
        memset(map->inline_buffer, 0, sizeof(map->inline_buffer)); /* Sets keys to PHM_EMPTY_SLOT_KEY */ \
    }                                                                           \
                                                                                \
    /* Inserts a *non-NULL* key into a hash table (heap or grown inline) */     \
    /* Assumes table has enough space and key is not NULL */                    \
    static void Name##_insert_entry(Name##_kv_pair* table, uint64_t capacity,   \
                                     void* key, ValueType value) {              \
        /* Assert(key != PHM_EMPTY_SLOT_KEY); */                                \
        uint64_t cap_mask = capacity - 1;                                       \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
        while (1) {                                                             \
            Name##_kv_pair* slot = &table[index];                               \
            /* Find the first empty slot */                                     \
            if (slot->key == PHM_EMPTY_SLOT_KEY) {                              \
                slot->key = key;                                                \
                slot->value = value;                                            \
                return;                                                         \
            }                                                                   \
            /* Linear probing */                                                \
            index = (index + 1) & cap_mask;                                     \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Grows the hash table (or transitions from inline) */                     \
    /* Preserves the NULL key state */                                          \
    static void Name##_grow(Name* map) {                                        \
        bool is_inline = (map->table == map->inline_buffer);                    \
        uint64_t old_non_null_size = map->current_size; /* Count of non-NULL keys */ \
        uint64_t old_capacity = is_inline ? PHM_INLINE_CAPACITY : map->current_capacity; \
        Name##_kv_pair* old_table_ptr = map->table;                             \
                                                                                \
        uint64_t new_capacity = (old_capacity < PHM_INITIAL_HEAP_CAPACITY)      \
                                  ? PHM_INITIAL_HEAP_CAPACITY                   \
                                  : old_capacity * 2;                           \
        /* Ensure capacity doesn't stay 0 if PHM_INITIAL_HEAP_CAPACITY is 0 */  \
        if (new_capacity == 0 && PHM_INITIAL_HEAP_CAPACITY > 0) {               \
             new_capacity = PHM_INITIAL_HEAP_CAPACITY;                          \
        }                                                                       \
        /* Handle potential allocation failure or 0 capacity */                 \
        if (new_capacity == 0) return; /* Cannot grow */                        \
                                                                                \
        Name##_kv_pair* new_table = (Name##_kv_pair*)calloc(new_capacity, sizeof(Name##_kv_pair)); \
        if (!new_table) return; /* Allocation failed */                         \
                                                                                \
        /* Rehash only non-NULL entries from the old table */                   \
        /* Note: Inline buffer isn't hashed, it's iterated linearly */          \
        uint64_t entries_to_rehash = is_inline ? old_non_null_size : old_capacity; \
        for (uint64_t i = 0; i < entries_to_rehash; ++i) {                      \
            Name##_kv_pair* entry = &old_table_ptr[i];                          \
            if (entry->key != PHM_EMPTY_SLOT_KEY) {                             \
                /* Key is guaranteed non-NULL here */                           \
                Name##_insert_entry(new_table, new_capacity, entry->key, entry->value); \
            }                                                                   \
        }                                                                       \
                                                                                \
        /* Free old heap table if it existed */                                 \
        if (!is_inline) free(old_table_ptr);                                    \
                                                                                \
        /* Update map state */                                                  \
        map->table = new_table;                                                 \
        map->current_capacity = new_capacity;                                   \
        /* current_size (non-NULL count) remains the same after rehash */       \
        map->current_size = old_non_null_size;                                  \
        /* has_null_key and null_value are preserved */                         \
    }                                                                           \
                                                                                \
    /* Finds the slot where a *non-NULL* key should reside */                   \
    /* Returns pointer to the slot (which might be empty or contain the key) */ \
    /* Assumes map is not inline and capacity > 0 */                            \
    static inline Name##_kv_pair* Name##_find_slot(Name* map, void* key) {      \
        /* Assert(key != NULL && map->table != map->inline_buffer && map->current_capacity > 0); */ \
        uint64_t cap_mask = map->current_capacity - 1;                          \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
        while (1) {                                                             \
            Name##_kv_pair* slot = &map->table[index];                          \
            /* Stop if slot is empty or contains the key */                     \
            if (slot->key == PHM_EMPTY_SLOT_KEY || slot->key == key) {          \
                return slot;                                                    \
            }                                                                   \
            /* Linear probing */                                                \
            index = (index + 1) & cap_mask;                                     \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Inserts or updates a key-value pair. Handles NULL key. */                \
    SCOPE void Name##_put(Name* map, void* key, ValueType value) {              \
        /* Handle NULL key separately */                                        \
        if (key == NULL) {                                                      \
            map->has_null_key = true;                                           \
            map->null_value = value;                                            \
            return;                                                             \
        }                                                                       \
                                                                                \
        /* Handle inline buffer case (linear scan) */                           \
        if (map->table == map->inline_buffer) {                                 \
            /* Check if key already exists */                                   \
            for (uint64_t i = 0; i < map->current_size; ++i) {                  \
                /* Note: map->current_size tracks non-NULL keys in inline buf */\
                if (map->inline_buffer[i].key == key) {                         \
                    map->inline_buffer[i].value = value; /* Update */           \
                    return;                                                     \
                }                                                               \
            }                                                                   \
            /* Key not found, check if there is space */                        \
            if (map->current_size < PHM_INLINE_CAPACITY) {                      \
                map->inline_buffer[map->current_size].key = key;                \
                map->inline_buffer[map->current_size].value = value;            \
                map->current_size++; /* Increment non-NULL count */             \
                return;                                                         \
            } else {                                                            \
                /* Inline buffer full, grow to heap */                          \
                Name##_grow(map);                                               \
                /* If grow failed (e.g., allocation failed), map->table might still be inline */ \
                /* Or new capacity might be 0 */                                \
                if (map->table == map->inline_buffer || map->current_capacity == 0) { \
                    /* Cannot insert if grow failed */                          \
                    return;                                                     \
                }                                                               \
                /* Fall through to heap insertion logic */                      \
            }                                                                   \
        }                                                                       \
                                                                                \
        /* Handle heap buffer case */                                           \
        /* Ensure capacity is sufficient *before* finding slot */               \
        /* current_size is non-NULL count */                                    \
        /* Check if we need to grow due to load factor */                       \
        if (map->current_capacity > 0 &&                                        \
            map->current_size * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM) { \
            Name##_grow(map);                                                   \
            /* Check if grow failed */                                          \
            if (map->current_capacity == 0) return;                             \
        }                                                                       \
        /* Handle edge case where capacity was 0 initially or after failed grow */ \
        if (map->current_capacity == 0) {                                       \
             /* This case implies inline buffer was not used or grow failed */  \
             /* Try growing again (e.g. from 0 capacity) */                    \
             Name##_grow(map);                                                  \
             if (map->current_capacity == 0) return; /* Still cannot insert */  \
        }                                                                       \
                                                                                \
        /* Find the slot for the non-NULL key in the heap table */              \
        Name##_kv_pair* slot = Name##_find_slot(map, key);                      \
                                                                                \
        /* Insert if slot is empty, otherwise update */                         \
        if (slot->key == PHM_EMPTY_SLOT_KEY) {                                  \
            slot->key = key;                                                    \
            slot->value = value;                                                \
            map->current_size++; /* Increment non-NULL count */                 \
        } else {                                                                \
            /* Key already exists, update value */                              \
            slot->value = value;                                                \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Gets the value associated with a key. Handles NULL key. */               \
    /* Returns NULL if key not found. */                                        \
    SCOPE ValueType* Name##_get(Name* map, void* key) {                         \
        /* Handle NULL key separately */                                        \
        if (key == NULL) {                                                      \
            return map->has_null_key ? &map->null_value : NULL;                 \
        }                                                                       \
                                                                                \
        /* Handle inline buffer case (linear scan) */                           \
        if (map->table == map->inline_buffer) {                                 \
            for (uint64_t i = 0; i < map->current_size; ++i) {                  \
                if (map->inline_buffer[i].key == key) {                         \
                    return &map->inline_buffer[i].value;                        \
                }                                                               \
            }                                                                   \
            return NULL; /* Not found in inline buffer */                       \
        }                                                                       \
                                                                                \
        /* Handle heap buffer case */                                           \
        if (map->current_capacity == 0) return NULL; /* Empty heap table */     \
                                                                                \
        Name##_kv_pair* slot = Name##_find_slot(map, key);                      \
        /* Check if the found slot actually contains the key */                 \
        return (slot->key == key) ? &slot->value : NULL;                        \
    }                                                                           \
                                                                                \
    /* Iterates over all key-value pairs, including NULL key if present. */     \
    SCOPE void Name##_for_each(Name* map, void (*callback)(void* key, ValueType* value, void* user_data), void* user_data) { \
        /* Handle NULL key first if present */                                  \
        if (map->has_null_key) {                                                \
            callback(NULL, &map->null_value, user_data);                        \
        }                                                                       \
                                                                                \
        /* Iterate through the current table (inline or heap) */                \
        if (map->table == map->inline_buffer) {                                 \
            /* Iterate only up to current_size for inline buffer */             \
            for (uint64_t i = 0; i < map->current_size; ++i) {                  \
                /* Keys here are guaranteed non-NULL */                         \
                 callback(map->inline_buffer[i].key, &map->inline_buffer[i].value, user_data); \
            }                                                                   \
        } else if (map->current_capacity > 0) {                                 \
            /* Iterate through the entire heap table */                         \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {              \
                if (map->table[i].key != PHM_EMPTY_SLOT_KEY) {                  \
                    /* Key is guaranteed non-NULL */                            \
                    callback(map->table[i].key, &map->table[i].value, user_data); \
                }                                                               \
            }                                                                   \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Frees heap memory if allocated and resets the map */                     \
    SCOPE void Name##_free(Name* map) {                                         \
        if (map->table != map->inline_buffer) {                                 \
            free(map->table);                                                   \
        }                                                                       \
        /* Re-initialize to reset all fields, including NULL key state */       \
        Name##_init(map);                                                       \
    }

#endif // PTR_HASH_MAP_GENERIC_H
#endif // PTR_HASH_MAP


// =============================================================================
// Pointer Hash Set Implementation (with NULL element support)
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
// PHM_EMPTY_SLOT_KEY is also implicitly reused (NULL)

// Macro to generate the hash set implementation
#define DEFINE_PTR_HASH_SET(SCOPE, Name)                                        \
                                                                                \
    typedef struct Name {                                                       \
        uint64_t current_size;    /* Number of *non-NULL* elements */           \
        bool has_null_element;  /* Flag indicating if NULL element is present */\
        void** table; /* Points to inline_buffer or heap (array of void*) */    \
                      /* PHM_EMPTY_SLOT_KEY (NULL) indicates empty slot */      \
        union {                                                                 \
            uint64_t current_capacity; /* Only valid when table is heap */      \
            void* inline_buffer[PHM_INLINE_CAPACITY]; /* Stores non-NULL ptrs */ \
        };                                                                      \
    } Name;                                                                     \
                                                                                \
    /* Hash function remains the same, only used for non-NULL keys */           \
    static inline uint64_t Name##_hash_key(void* key) {                         \
        /* Assert(key != NULL) could be added here */                           \
        return (uintptr_t)key * PHM_HASH_MULTIPLIER;                            \
    }                                                                           \
                                                                                \
    static void Name##_grow(Name* set);                                         \
                                                                                \
    /* Initializes the set, including NULL element state */                     \
    SCOPE void Name##_init(Name* set) {                                         \
        set->current_size = 0;                                                  \
        set->has_null_element = false;                                          \
        set->table = set->inline_buffer;                                        \
        /* Initialize inline buffer slots to empty */                           \
        memset(set->inline_buffer, 0, sizeof(set->inline_buffer)); /* Sets slots to PHM_EMPTY_SLOT_KEY */ \
    }                                                                           \
                                                                                \
    /* Inserts a *non-NULL* key into a hash table (heap or grown inline) */     \
    /* Assumes table has enough space and key is not NULL */                    \
    static void Name##_insert_key(void** table, uint64_t capacity, void* key) { \
        /* Assert(key != PHM_EMPTY_SLOT_KEY); */                                \
        uint64_t cap_mask = capacity - 1;                                       \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
        while (1) {                                                             \
            void** slot_ptr = &table[index];                                    \
            /* Find the first empty slot */                                     \
            if (*slot_ptr == PHM_EMPTY_SLOT_KEY) {                              \
                *slot_ptr = key; /* Store the non-NULL key */                   \
                return;                                                         \
            }                                                                   \
            /* Linear probing */                                                \
            index = (index + 1) & cap_mask;                                     \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Grows the hash table (or transitions from inline) */                     \
    /* Preserves the NULL element state */                                      \
    static void Name##_grow(Name* set) {                                        \
        bool is_inline = (set->table == set->inline_buffer);                    \
        uint64_t old_non_null_size = set->current_size; /* Count of non-NULL elements */ \
        uint64_t old_capacity = is_inline ? PHM_INLINE_CAPACITY : set->current_capacity; \
        void** old_table_ptr = set->table;                                      \
                                                                                \
        uint64_t new_capacity = (old_capacity < PHM_INITIAL_HEAP_CAPACITY)      \
                                  ? PHM_INITIAL_HEAP_CAPACITY                   \
                                  : old_capacity * 2;                           \
        if (new_capacity == 0 && PHM_INITIAL_HEAP_CAPACITY > 0) {               \
             new_capacity = PHM_INITIAL_HEAP_CAPACITY;                          \
        }                                                                       \
        if (new_capacity == 0) return; /* Cannot grow */                        \
                                                                                \
        void** new_table = (void**)calloc(new_capacity, sizeof(void*));         \
        if (!new_table) return; /* Allocation failed */                         \
                                                                                \
        /* Rehash only non-NULL elements from the old table */                  \
        uint64_t entries_to_rehash = is_inline ? old_non_null_size : old_capacity; \
        for (uint64_t i = 0; i < entries_to_rehash; ++i) {                      \
            void* key = old_table_ptr[i];                                       \
            if (key != PHM_EMPTY_SLOT_KEY) {                                    \
                /* Key is guaranteed non-NULL here */                           \
                Name##_insert_key(new_table, new_capacity, key);                \
            }                                                                   \
        }                                                                       \
                                                                                \
        /* Free old heap table if it existed */                                 \
        if (!is_inline) free(old_table_ptr);                                    \
                                                                                \
        /* Update set state */                                                  \
        set->table = new_table;                                                 \
        set->current_capacity = new_capacity;                                   \
        /* current_size (non-NULL count) remains the same after rehash */       \
        set->current_size = old_non_null_size;                                  \
        /* has_null_element is preserved */                                     \
    }                                                                           \
                                                                                \
    /* Finds the slot where a *non-NULL* key should reside */                   \
    /* Returns pointer to the slot pointer (which might point to empty or the key) */ \
    /* Assumes set is not inline and capacity > 0 */                            \
    static inline void** Name##_find_slot(Name* set, void* key) {               \
        /* Assert(key != NULL && set->table != set->inline_buffer && set->current_capacity > 0); */ \
        uint64_t cap_mask = set->current_capacity - 1;                          \
        uint64_t h = Name##_hash_key(key);                                      \
        uint64_t index = h & cap_mask;                                          \
        while (1) {                                                             \
            void** slot_ptr = &set->table[index];                               \
            /* Stop if slot is empty or contains the key */                     \
            if (*slot_ptr == PHM_EMPTY_SLOT_KEY || *slot_ptr == key) {          \
                return slot_ptr;                                                \
            }                                                                   \
            /* Linear probing */                                                \
            index = (index + 1) & cap_mask;                                     \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Adds an element to the set. Handles NULL element. */                     \
    /* Returns true if the element was added, false if it was already present. */\
    SCOPE bool Name##_add(Name* set, void* key) {                               \
        /* Handle NULL element separately */                                    \
        if (key == NULL) {                                                      \
            if (set->has_null_element) {                                        \
                return false; /* NULL already present */                        \
            } else {                                                            \
                set->has_null_element = true;                                   \
                return true; /* Added NULL */                                   \
            }                                                                   \
        }                                                                       \
                                                                                \
        /* Handle inline buffer case (linear scan) */                           \
        if (set->table == set->inline_buffer) {                                 \
            /* Check if key already exists */                                   \
            for (uint64_t i = 0; i < set->current_size; ++i) {                  \
                /* current_size tracks non-NULL elements */                     \
                if (set->inline_buffer[i] == key) {                             \
                    return false; /* Key already present */                     \
                }                                                               \
            }                                                                   \
            /* Key not found, check if there is space */                        \
            if (set->current_size < PHM_INLINE_CAPACITY) {                      \
                set->inline_buffer[set->current_size] = key;                    \
                set->current_size++; /* Increment non-NULL count */             \
                return true; /* Added non-NULL key */                           \
            } else {                                                            \
                /* Inline buffer full, grow to heap */                          \
                Name##_grow(set);                                               \
                if (set->table == set->inline_buffer || set->current_capacity == 0) { \
                    /* Cannot insert if grow failed */                          \
                    return false; /* Indicate failure or element not added */   \
                }                                                               \
                /* Fall through to heap insertion logic */                      \
            }                                                                   \
        }                                                                       \
                                                                                \
        /* Handle heap buffer case */                                           \
        /* Ensure capacity is sufficient */                                     \
        if (set->current_capacity > 0 &&                                        \
            set->current_size * PHM_LOAD_FACTOR_DEN >= set->current_capacity * PHM_LOAD_FACTOR_NUM) { \
            Name##_grow(set);                                                   \
            if (set->current_capacity == 0) return false; /* Grow failed */     \
        }                                                                       \
        if (set->current_capacity == 0) {                                       \
             Name##_grow(set);                                                  \
             if (set->current_capacity == 0) return false; /* Still cannot insert */ \
        }                                                                       \
                                                                                \
        /* Find the slot for the non-NULL key */                                \
        void** slot_ptr = Name##_find_slot(set, key);                           \
                                                                                \
        /* Add if slot is empty */                                              \
        if (*slot_ptr == PHM_EMPTY_SLOT_KEY) {                                  \
            *slot_ptr = key;                                                    \
            set->current_size++; /* Increment non-NULL count */                 \
            return true; /* Added */                                            \
        } else {                                                                \
            /* Key already present */                                           \
            return false;                                                       \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Checks if an element is present in the set. Handles NULL element. */     \
    SCOPE bool Name##_contains(Name* set, void* key) {                          \
        /* Handle NULL element separately */                                    \
        if (key == NULL) {                                                      \
            return set->has_null_element;                                       \
        }                                                                       \
                                                                                \
        /* Handle inline buffer case (linear scan) */                           \
        if (set->table == set->inline_buffer) {                                 \
            for (uint64_t i = 0; i < set->current_size; ++i) {                  \
                if (set->inline_buffer[i] == key) {                             \
                    return true;                                                \
                }                                                               \
            }                                                                   \
            return false; /* Not found in inline buffer */                      \
        }                                                                       \
                                                                                \
        /* Handle heap buffer case */                                           \
        if (set->current_capacity == 0) return false; /* Empty heap table */    \
                                                                                \
        void** slot_ptr = Name##_find_slot(set, key);                           \
        /* Check if the found slot actually contains the key */                 \
        return (*slot_ptr == key);                                              \
    }                                                                           \
                                                                                \
    /* Iterates over all elements, including NULL element if present. */        \
    SCOPE void Name##_for_each(Name* set, void (*callback)(void* key, void* user_data), void* user_data) { \
        /* Handle NULL element first if present */                              \
        if (set->has_null_element) {                                            \
            callback(NULL, user_data);                                          \
        }                                                                       \
                                                                                \
        /* Iterate through the current table (inline or heap) */                \
        if (set->table == set->inline_buffer) {                                 \
            /* Iterate only up to current_size for inline buffer */             \
            for (uint64_t i = 0; i < set->current_size; ++i) {                  \
                /* Elements here are guaranteed non-NULL */                     \
                callback(set->inline_buffer[i], user_data);                     \
            }                                                                   \
        } else if (set->current_capacity > 0) {                                 \
            /* Iterate through the entire heap table */                         \
            for (uint64_t i = 0; i < set->current_capacity; ++i) {              \
                if (set->table[i] != PHM_EMPTY_SLOT_KEY) {                      \
                    /* Element is guaranteed non-NULL */                        \
                    callback(set->table[i], user_data);                         \
                }                                                               \
            }                                                                   \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Frees heap memory if allocated and resets the set */                     \
    SCOPE void Name##_free(Name* set) {                                         \
        if (set->table != set->inline_buffer) {                                 \
            free(set->table);                                                   \
        }                                                                       \
        /* Re-initialize to reset all fields, including NULL element state */   \
        Name##_init(set);                                                       \
    }

#endif // PTR_HASH_SET_GENERIC_H


#define DEFINE_VECTOR(SCOPE, Type, Name) \
\
typedef struct { \
    Type *data; \
    size_t size; \
    size_t capacity; \
} Name; \
\
SCOPE void Name##_init(Name *v) { \
    v->data = NULL; \
    v->size = 0; \
    v->capacity = 0; \
} \
\
SCOPE void Name##_free(Name *v) { \
    free(v->data); \
    v->data = NULL; /* Optional: prevent double free */ \
    v->size = 0; \
    v->capacity = 0; \
} \
\
SCOPE int Name##_reserve(Name *v, size_t new_capacity) { \
    if (new_capacity <= v->capacity) { \
        return 0; \
    } \
    Type *new_data = (Type *)realloc(v->data, new_capacity * sizeof(Type)); \
    if (!new_data && new_capacity > 0) { \
        /* realloc failed, vector remains unchanged */ \
        return -1; \
    } \
    v->data = new_data; \
    v->capacity = new_capacity; \
    return 0; \
} \
\
SCOPE int Name##_push_back(Name *v, Type value) { \
    if (v->size >= v->capacity) { \
        size_t new_capacity = (v->capacity == 0) ? 1 : v->capacity * 2; \
        /* Check for potential size_t overflow */ \
        if (new_capacity < v->capacity) { \
             /* Overflow occurred, cannot allocate more memory safely */ \
             return -1; \
        } \
        if (Name##_reserve(v, new_capacity) != 0) { \
            return -1; \
        } \
    } \
    v->data[v->size++] = value; \
    return 0; \
} \
\
/* Optional: Add _get, _set, _pop_back, _clear etc. if needed */ \
/* Example: Get element (no bounds check for brevity) */ \
SCOPE Type* Name##_get(Name *v, size_t index) { \
    /* assert(index < v->size); // Recommended for debugging */ \
    return &v->data[index]; \
} \
\
/* Example: Get size */ \
SCOPE size_t Name##_size(const Name *v) { \
    return v->size; \
}


/*
// Example Usage:
// Needs to be included *after* the DEFINE_VECTOR macro definition

#include <stdio.h> // For printf in example

// Define a vector of integers named IntVec
DEFINE_VECTOR(static inline, int, IntVec)

int main() {
    IntVec my_vector;
    IntVec_init(&my_vector);

    // Push some elements
    for (int i = 0; i < 10; ++i) {
        if (IntVec_push_back(&my_vector, i * 10) != 0) {
            fprintf(stderr, "Failed to push_back %d\n", i * 10);
            IntVec_free(&my_vector);
            return 1;
        }
        printf("Pushed %d, Size: %zu, Capacity: %zu\n",
               i * 10, my_vector.size, my_vector.capacity);
    }

    printf("\nVector elements:\n");
    for (size_t i = 0; i < IntVec_size(&my_vector); ++i) {
        // Direct access or use _get
        // printf("%d ", my_vector.data[i]);
        printf("%d ", *IntVec_get(&my_vector, i));
    }
    printf("\n");

    IntVec_free(&my_vector);
    return 0;
}
*/

#endif // PTR_HASH_SET