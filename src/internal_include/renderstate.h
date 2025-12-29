// begin file src/internal_include/renderstate.h
#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include <stdlib.h>
#include <string.h>
#include <macros_and_constants.h>
#include <raygpu.h>
#if SUPPORT_WGPU_BACKEND == 1
#include <webgpu/webgpu.h>
#else
#include <wgvk.h>
#endif



#define DEFINE_ARRAY_STACK(T, LINKAGE, N)                                       \
typedef struct {                                                                \
    T data[N];                                                                  \
    uint32_t current_pos;                                                       \
} T##_stack;                                                                    \
LINKAGE void T##_stack_init(T##_stack *s) { s->current_pos = 0; }               \
LINKAGE void T##_stack_push(T##_stack *s, T v) {                                \
    s->data[s->current_pos++] = v;                                              \
}                                                                               \
LINKAGE T T##_stack_pop(T##_stack *s) {                                         \
    return s->data[--s->current_pos];                                           \
}                                                                               \
LINKAGE T *T##_stack_peek(T##_stack *s) {                                       \
    return &s->data[s->current_pos - 1];                                        \
}                                                                               \
LINKAGE const T *T##_stack_cpeek(const T##_stack *s) {                          \
    return &s->data[s->current_pos - 1];                                        \
}                                                                               \
LINKAGE size_t T##_stack_size(const T##_stack *s) { return s->current_pos; }    \
LINKAGE int T##_stack_empty(const T##_stack *s) { return s->current_pos == 0; }
typedef struct MatrixBufferPair{
    Matrix matrix;
    WGPUBuffer buffer;
}MatrixBufferPair;

DEFINE_ARRAY_STACK(MatrixBufferPair, static inline, 8);
DEFINE_ARRAY_STACK(RenderTexture, static inline, 8);
// public config knobs with safe defaults
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

#ifndef PHM_EMPTY_SLOT_KEY
#define PHM_EMPTY_SLOT_KEY NULL
#endif

#ifndef PHM_DELETED_SLOT_KEY
#define PHM_DELETED_SLOT_KEY ((void*)0xFFFFFFFFFFFFFFFFULL)
#endif

#define DEFINE_VECTOR_IW(SCOPE, Type, Name)                                              \
                                                                                         \
    typedef struct {                                                                     \
        Type *data;                                                                      \
        size_t size;                                                                     \
        size_t capacity;                                                                 \
    } Name;                                                                              \
                                                                                         \
    SCOPE void Name##_init(Name *v) {                                                    \
        v->data = NULL;                                                                  \
        v->size = 0;                                                                     \
        v->capacity = 0;                                                                 \
    }                                                                                    \
                                                                                         \
    SCOPE bool Name##_empty(Name *v) { return v->size == 0; }                            \
                                                                                         \
    SCOPE void Name##_initWithSize(Name *v, size_t initialSize) {                        \
        if (initialSize == 0) {                                                          \
            Name##_init(v);                                                              \
        } else {                                                                         \
            v->data = (Type *)calloc(initialSize, sizeof(Type));                         \
            if (!v->data) {                                                              \
                Name##_init(v);                                                          \
                return;                                                                  \
            }                                                                            \
            v->size = initialSize;                                                       \
            v->capacity = initialSize;                                                   \
        }                                                                                \
    }                                                                                    \
                                                                                         \
    SCOPE void Name##_free(Name *v) {                                                    \
        if (v->data != NULL) {                                                           \
            free((void*)v->data);                                                        \
        }                                                                                \
        Name##_init(v);                                                                  \
    }                                                                                    \
    SCOPE void Name##_clear(Name *v) { v->size = 0; }                                    \
    SCOPE int Name##_reserve(Name *v, size_t new_capacity) {                             \
        if (new_capacity <= v->capacity) {                                               \
            return 0;                                                                    \
        }                                                                                \
        Type *new_data = (Type *)realloc((void*)v->data, new_capacity * sizeof(Type));   \
        if (!new_data && new_capacity > 0) {                                             \
            return -1;                                                                   \
        }                                                                                \
        v->data = new_data;                                                              \
        v->capacity = new_capacity;                                                      \
        return 0;                                                                        \
    }                                                                                    \
                                                                                         \
    SCOPE int Name##_push_back(Name *v, Type value) {                                    \
        if (v->size >= v->capacity) {                                                    \
            size_t new_capacity;                                                         \
            if (v->capacity == 0) {                                                      \
                new_capacity = 8;                                                        \
            } else {                                                                     \
                new_capacity = v->capacity * 2;                                          \
            }                                                                            \
            if (new_capacity < v->capacity) {                                            \
                return -1;                                                               \
            }                                                                            \
            if (Name##_reserve(v, new_capacity) != 0) {                                  \
                return -1;                                                               \
            }                                                                            \
        }                                                                                \
        v->data[v->size++] = value;                                                      \
        return 0;                                                                        \
    }                                                                                    \
    SCOPE void Name##_pop_back(Name *v) { --v->size; }                                   \
    SCOPE Type *Name##_get(Name *v, size_t index) {                                      \
        if (index >= v->size) {                                                          \
            return NULL;                                                                 \
        }                                                                                \
        return &v->data[index];                                                          \
    }                                                                                    \
                                                                                         \
    SCOPE size_t Name##_size(const Name *v) { return v->size; }                          \
                                                                                         \
    SCOPE void Name##_move(Name *dest, Name *source) {                                   \
        if (dest == source) {                                                            \
            return;                                                                      \
        }                                                                                \
        dest->data = source->data;                                                       \
        dest->size = source->size;                                                       \
        dest->capacity = source->capacity;                                               \
        Name##_init(source);                                                             \
    }                                                                                    \
                                                                                         \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                             \
        Name##_free(dest);                                                               \
                                                                                         \
        if (source->size == 0) {                                                         \
            return;                                                                      \
        }                                                                                \
                                                                                         \
        size_t capacity_to_allocate = source->capacity;                                  \
        if (capacity_to_allocate < source->size) {                                       \
            capacity_to_allocate = source->size;                                         \
        }                                                                                \
                                                                                         \
        dest->data = (Type*)malloc(capacity_to_allocate * sizeof(Type));                 \
        if (!dest->data) {                                                               \
            return;                                                                      \
        }                                                                                \
        memcpy((void*)dest->data, (void*)source->data, source->size * sizeof(Type));     \
        dest->size = source->size;                                                       \
        dest->capacity = capacity_to_allocate;                                           \
    }



#define DEFINE_PTR_HASH_MAP_R(SCOPE, Name, ValueType)                                                                              \
    typedef struct Name##_kv_pair {                                                                                                \
        void   *key;                                                                                                               \
        ValueType value;                                                                                                           \
    } Name##_kv_pair;                                                                                                              \
    typedef struct Name {                                                                                                          \
        uint64_t        current_size;                                                                                              \
        uint64_t        current_capacity;                                                                                          \
        bool            has_null_key;                                                                                              \
        ValueType       null_value;                                                                                                \
        Name##_kv_pair *table;                                                                                                     \
    } Name;                                                                                                                        \
    static inline uint64_t Name##_hash_key(void *key) {                                                                            \
        return ((uintptr_t)key) * (uint64_t)PHM_HASH_MULTIPLIER;                                                                   \
    }                                                                                                                              \
    /* next power of two, returns 0 on v==0 */                                                                                     \
    static inline uint64_t Name##_round_up_to_power_of_2(uint64_t v) {                                                             \
        if (v == 0) return 0;                                                                                                      \
        v--;                                                                                                                       \
        v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v |= v >> 32;                                            \
        v++;                                                                                                                       \
        return v;                                                                                                                  \
    }                                                                                                                              \
    static inline uint64_t Name##_find_index_for_get(const Name *map, void *key) {                                                 \
        uint64_t mask = map->current_capacity - 1;                                                                                 \
        uint64_t i = Name##_hash_key(key) & mask;                                                                                  \
        for (;;) {                                                                                                                 \
            void *k = map->table[i].key;                                                                                           \
            if (k == PHM_EMPTY_SLOT_KEY) return i; /* not found */                                                                 \
            if (k == key) return i;                                                                                                \
            i = (i + 1) & mask;                                                                                                    \
        }                                                                                                                          \
    }                                                                                                                              \
                                                                                                                                \
    /* internal: probe to find slot to insert/update. prefers first tombstone if seen */                                           \
    static inline uint64_t Name##_find_index_for_put(const Name *map, void *key) {                                                 \
        uint64_t mask = map->current_capacity - 1;                                                                                 \
        uint64_t i = Name##_hash_key(key) & mask;                                                                                  \
        uint64_t first_tomb = UINT64_MAX;                                                                                          \
        for (;;) {                                                                                                                 \
            void *k = map->table[i].key;                                                                                           \
            if (k == PHM_EMPTY_SLOT_KEY) {                                                                                         \
                return (first_tomb != UINT64_MAX) ? first_tomb : i;                                                                \
            }                                                                                                                      \
            if (k == key) return i;                                                                                                \
            if (k == PHM_DELETED_SLOT_KEY && first_tomb == UINT64_MAX) first_tomb = i;                                             \
            i = (i + 1) & mask;                                                                                                    \
        }                                                                                                                          \
    }                                                                                                                              \
                                                                                                                                   \
    static void Name##_rehash_into(Name##_kv_pair *dst, uint64_t dst_cap, const Name##_kv_pair *src, uint64_t src_cap) {           \
        uint64_t mask = dst_cap - 1;                                                                                               \
        for (uint64_t i = 0; i < src_cap; ++i) {                                                                                   \
            void *k = src[i].key;                                                                                                  \
            if (k == PHM_EMPTY_SLOT_KEY || k == PHM_DELETED_SLOT_KEY) continue;                                                    \
            uint64_t j = (((uintptr_t)k) * (uint64_t)PHM_HASH_MULTIPLIER) & mask;                                                  \
            while (dst[j].key != PHM_EMPTY_SLOT_KEY) j = (j + 1) & mask;                                                           \
            dst[j].key = k;                                                                                                        \
            dst[j].value = src[i].value;                                                                                           \
        }                                                                                                                          \
    }                                                                                                                              \
                                                                                                                                   \
    static void Name##_grow(Name *map) {                                                                                           \
        uint64_t new_cap = (map->current_capacity == 0) ? PHM_INITIAL_HEAP_CAPACITY : (map->current_capacity << 1);                \
        if (new_cap < PHM_INITIAL_HEAP_CAPACITY) new_cap = PHM_INITIAL_HEAP_CAPACITY;                                              \
        new_cap = Name##_round_up_to_power_of_2(new_cap);                                                                          \
        if (new_cap == 0) new_cap = PHM_INITIAL_HEAP_CAPACITY;                                                                     \
        Name##_kv_pair *new_tab = (Name##_kv_pair*)calloc(new_cap, sizeof(Name##_kv_pair));                                        \
        if (!new_tab) abort();                                                                                                     \
        /* calloc zeroes keys to NULL which is PHM_EMPTY_SLOT_KEY */                                                               \
        if (map->table && map->current_capacity) {                                                                                 \
            Name##_rehash_into(new_tab, new_cap, map->table, map->current_capacity);                                               \
            free(map->table);                                                                                                      \
        }                                                                                                                          \
        map->table = new_tab;                                                                                                      \
        map->current_capacity = new_cap;                                                                                           \
    }                                                                                                                              \
                                                                                                                                   \
    static inline bool Name##_should_grow(const Name *map, uint64_t add) {                                                         \
        /* trigger when (size + add) / capacity > NUM/DEN. capacity==0 always grows. */                                            \
        if (map->current_capacity == 0) return true;                                                                               \
        uint64_t future = map->current_size + add;                                                                                 \
        return (future * (uint64_t)PHM_LOAD_FACTOR_DEN) > (map->current_capacity * (uint64_t)PHM_LOAD_FACTOR_NUM);                 \
    }                                                                                                                              \
                                                                                                                                   \
    SCOPE void Name##_init(Name *map) {                                                                                            \
        map->current_size = 0;                                                                                                     \
        map->current_capacity = 0;                                                                                                 \
        map->has_null_key = false;                                                                                                 \
        /* map->null_value left uninitialized until set */                                                                         \
        map->table = NULL;                                                                                                         \
    }                                                                                                                              \
                                                                                                                                   \
    SCOPE int Name##_put(Name *map, void *key, ValueType value) {                                                                  \
        if (key == NULL) {                                                                                                         \
            int inserted = map->has_null_key ? 0 : 1;                                                                              \
            map->null_value = value;                                                                                               \
            map->has_null_key = true;                                                                                              \
            return inserted;                                                                                                       \
        }                                                                                                                          \
        if (Name##_should_grow(map, 1)) Name##_grow(map);                                                                          \
        uint64_t idx = Name##_find_index_for_put(map, key);                                                                        \
        void *k = map->table[idx].key;                                                                                             \
        int inserted = (k != key);                                                                                                 \
        if (inserted) {                                                                                                            \
            if (k == PHM_EMPTY_SLOT_KEY || k == PHM_DELETED_SLOT_KEY) {                                                            \
                map->current_size++;                                                                                               \
            }                                                                                                                      \
            map->table[idx].key = key;                                                                                             \
        }                                                                                                                          \
        map->table[idx].value = value;                                                                                             \
        return inserted;                                                                                                           \
    }                                                                                                                              \
                                                                                                                                   \
    SCOPE ValueType* Name##_get(Name *map, void *key) {                                                                            \
        if (key == NULL) {                                                                                                         \
            if (!map->has_null_key) return NULL;                                                                                   \
            return &map->null_value;                                                                                               \
        }                                                                                                                          \
        if (map->current_capacity == 0 || map->table == NULL) return NULL;                                                         \
        uint64_t idx = Name##_find_index_for_get(map, key);                                                                        \
        if (map->table[idx].key != key) return NULL;                                                                               \
        return &map->table[idx].value;                                                                                             \
    }                                                                                                                              \
                                                                                                                                   \
    SCOPE bool Name##_erase(Name *map, void *key) {                                                                                \
        if (key == NULL) {                                                                                                         \
            bool had = map->has_null_key;                                                                                          \
            map->has_null_key = false;                                                                                             \
            return had;                                                                                                            \
        }                                                                                                                          \
        if (map->current_capacity == 0 || map->table == NULL) return false;                                                        \
        uint64_t idx = Name##_find_index_for_get(map, key);                                                                        \
        if (map->table[idx].key != key) return false;                                                                              \
        map->table[idx].key = PHM_DELETED_SLOT_KEY;                                                                                \
        map->current_size--;                                                                                                       \
        return true;                                                                                                               \
    }                                                                                                                              \
                                                                                                                                   \
    SCOPE void Name##_free(Name *map) {                                                                                            \
        if (map->table) {                                                                                                          \
            free(map->table);                                                                                                      \
        }                                                                                                                          \
        map->table = NULL;                                                                                                         \
        map->current_capacity = 0;                                                                                                 \
        map->current_size = 0;                                                                                                     \
        map->has_null_key = false;                                                                                                 \
    }



DEFINE_PTR_HASH_MAP_R(static inline, CreatedWindowMap, RGWindowImpl)
DEFINE_VECTOR_IW(static inline, DescribedBuffer*, DescribedBufferVector)




typedef struct renderstate {
    WGPUPresentMode unthrottled_PresentMode;
    WGPUPresentMode throttled_PresentMode;

    GLFWwindow *window;
    uint32_t width, height;

    PixelFormat frameBufferFormat;

    Shader defaultShader;
    
    RenderSettings currentSettings;

    Shader activeShader;

    DescribedRenderpass clearPass;
    DescribedRenderpass renderpass;
    DescribedComputepass computepass;
    DescribedRenderpass *activeRenderpass;
    DescribedComputepass *activeComputepass;

    uint32_t renderExtentX;
    uint32_t renderExtentY;

    DescribedBufferVector smallBufferPool;
    DescribedBufferVector smallBufferRecyclingBin;

    DescribedBuffer *identityMatrix;
    DescribedSampler defaultSampler;

    DescribedBuffer *quadindicesCache;

    Texture whitePixel;

    MatrixBufferPair_stack matrixStack;
    RenderTexture_stack renderTargetStack;

    bool wantsToggleFullscreen;
    bool minimized;

    RenderTexture mainWindowRenderTarget;


    int windowFlags;
    // Frame timing / FPS
    int targetFPS;
    uint64_t total_frames;
    uint64_t init_timestamp;

    int64_t last_timestamps[64];

    GIFRecordState *grst;

    SubWindow mainWindow;
    CreatedWindowMap createdSubwindows;
    SubWindow activeSubWindow;
} renderstate;
#endif

// end file src/internal_include/renderstate.h