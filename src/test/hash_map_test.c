#include <stdio.h>
#include <assert.h>
#include <math.h> // For fabsf
#include <stdbool.h>
#include <stdlib.h> // For rand, srand, malloc, free
#include <time.h>   // For time (seeding rand)
#include <stdint.h> // For uintptr_t

// --- Include the modified header ---
// Assuming you saved the updated code as "ptr_hash_tables_generic.h"
#include <ptr_hash_map.h>
// --- ---

// Use external cc library (optional, remove if not using)
// #define CC_NO_SHORT_NAMES
// #include <external/cc.h>


// --- Define Specific Types ---
DEFINE_PTR_HASH_MAP(static inline, IntMap, int)
DEFINE_PTR_HASH_MAP(static inline, FloatMap, float)
DEFINE_PTR_HASH_SET(static inline, PtrSet) // Define the pointer set type
DEFINE_VECTOR(static inline, uint64_t, IntVector) // Define the pointer set type
// --- Helpers ---
static inline bool float_eq(float a, float b) {
    return fabsf(a - b) < 1e-6f;
}

// Generate a non-NULL pseudo-random pointer key for stress tests
static inline void* generate_random_key() {
    uintptr_t random_key_val = ((uintptr_t)rand() << 31) ^ ((uintptr_t)rand()); // Wider range
    // Ensure key is not NULL
    if (random_key_val == 0) random_key_val = 1; // Use 1 if 0 is generated
    return (void*)random_key_val;
}

// --- _for_each Callback Helpers ---

// For IntMap _for_each tests
typedef struct {
    int count;
    bool found_null;
    bool found_key1;
    bool found_key2;
    int null_value;
    int key1_value;
    int key2_value;
} IntMapForEachData;

static void intmap_for_each_callback(void* key, int* value, void* user_data) {
    IntMapForEachData* data = (IntMapForEachData*)user_data;
    data->count++;
    if (key == NULL) {
        data->found_null = true;
        data->null_value = *value;
    } else if (key == (void*)0x100) { // Corresponds to key1
        data->found_key1 = true;
        data->key1_value = *value;
    } else if (key == (void*)0x200) { // Corresponds to key2
        data->found_key2 = true;
        data->key2_value = *value;
    }
    // Add checks for other keys if needed for specific tests
}

// For PtrSet _for_each tests
typedef struct {
    int count;
    bool found_null;
    bool found_key1;
    bool found_key2;
} PtrSetForEachData;

static void ptrset_for_each_callback(void* key, void* user_data) {
    PtrSetForEachData* data = (PtrSetForEachData*)user_data;
    data->count++;
    if (key == NULL) {
        data->found_null = true;
    } else if (key == (void*)0x100) { // Corresponds to key1
        data->found_key1 = true;
    } else if (key == (void*)0x200) { // Corresponds to key2
        data->found_key2 = true;
    }
    // Add checks for other keys if needed
}


// --- Stress Tests (Unchanged - Focus on non-NULL hashing/resizing) ---

void run_intmap_stress_test(int num_elements) {
    printf("\n--- Running IntMap Stress Test (%d non-NULL elements) ---\n", num_elements);

    IntMap stress_map;
    IntMap_init(&stress_map);

    // Test adding NULL before stress
    printf("Adding NULL key before stress...\n");
    IntMap_put(&stress_map, NULL, -99);
    assert(stress_map.has_null_key);
    assert(stress_map.current_size == 0);
    int *null_val_ptr = IntMap_get(&stress_map, NULL);
    assert(null_val_ptr != NULL && *null_val_ptr == -99);


    void** keys_inserted = malloc(sizeof(void*) * num_elements);
    if (!keys_inserted) {
        fprintf(stderr, "Stress test memory allocation failed.\n");
        IntMap_free(&stress_map);
        return;
    }

    srand((unsigned int)time(NULL)); // Seed random number generator

    printf("Inserting %d non-NULL elements...\n", num_elements);
    int insertion_attempts = 0;
    uint64_t expected_non_null_size = 0; // Track unique non-NULL keys added

    // Use a temporary set to track unique keys added during the test *for this specific test run*
    // This helps verify the final map size more accurately if rand() generates duplicates.
    PtrSet keys_actually_added_set;
    PtrSet_init(&keys_actually_added_set);

    for (int i = 0; i < num_elements; ++i) {
        void* key = generate_random_key(); // generate_random_key ensures non-NULL
        int value = i + 1;

        keys_inserted[insertion_attempts] = key;
        insertion_attempts++;

        // Track if this non-NULL key is new for size verification
        if (!PtrSet_contains(&keys_actually_added_set, key)) {
             PtrSet_add(&keys_actually_added_set, key);
             expected_non_null_size++;
        }

        IntMap_put(&stress_map, key, value); // Put the non-NULL key

        // Check NULL key is still present and correct
        assert(stress_map.has_null_key);
        null_val_ptr = IntMap_get(&stress_map, NULL);
        assert(null_val_ptr != NULL && *null_val_ptr == -99);
        // Verify size reflects only non-NULL keys
        assert(stress_map.current_size == expected_non_null_size);


        if (num_elements <= 10000 || (i + 1) % (num_elements / 20) == 0) {
             bool is_inline = (stress_map.table == stress_map.inline_buffer);
             // Capacity calculation needs care if inline buffer is involved after init
             uint64_t capacity = (stress_map.table == stress_map.inline_buffer) ? PHM_INLINE_CAPACITY : stress_map.current_capacity;
             printf("... attempted %d non-NULL inserts (map non-NULL size: %llu, unique expected: %llu, capacity: %llu, %s, NULL present: %d)\n",
                    i + 1, (unsigned long long)stress_map.current_size, (unsigned long long)expected_non_null_size,
                    (unsigned long long)capacity, (stress_map.table == stress_map.inline_buffer) ? "inline" : "heap", stress_map.has_null_key);
        }
    }

    PtrSet_free(&keys_actually_added_set); // Free the temporary tracking set

    uint64_t final_non_null_size = stress_map.current_size;
    printf("Insertion complete. Attempted non-NULL insertions: %d. Final unique non-NULL map size: %llu (Expected unique: %llu). NULL key present: %d\n",
           insertion_attempts, (unsigned long long)final_non_null_size, (unsigned long long)expected_non_null_size, stress_map.has_null_key);
    assert(final_non_null_size == expected_non_null_size); // Final size should match unique non-NULL keys tracked

    printf("Verifying %d inserted non-NULL key-value pairs...\n", insertion_attempts);
    int verified_count = 0;
    int not_found = 0;

    // Check NULL key first
    null_val_ptr = IntMap_get(&stress_map, NULL);
    assert(null_val_ptr != NULL && *null_val_ptr == -99);

    // Verify presence of all *attempted* non-NULL keys
    for (int i = 0; i < insertion_attempts; ++i) {
        void* key_to_check = keys_inserted[i];
        assert(key_to_check != NULL); // Should be non-NULL from generator

        int* found_value_ptr = IntMap_get(&stress_map, key_to_check);

        if (found_value_ptr != NULL) {
            verified_count++;
            // Value check is tricky due to duplicates, presence is the main check
        } else {
            fprintf(stderr, "ERROR: Non-NULL Key %p (attempt index %d) was put but not found!\n", key_to_check, i);
            not_found++;
        }
        if (num_elements <= 10000 || (i + 1) % (insertion_attempts / 10) == 0 || insertion_attempts < 20) {
             printf("... verified presence for non-NULL attempt %d\n", i + 1);
        }
    }

    printf("Verification complete.\n");
    printf("Total attempted non-NULL insertions: %d\n", insertion_attempts);
    printf("Final non-NULL map size (unique keys): %llu\n", (unsigned long long)final_non_null_size);
    printf("Non-NULL Keys successfully found during verification: %d\n", verified_count);
    printf("Non-NULL Keys not found during verification: %d\n", not_found);
    printf("Has null key: %s\n", stress_map.has_null_key ? "true" : "false");
    printf("NULL key present and verified: %s\n", (stress_map.has_null_key && IntMap_get(&stress_map, NULL) != NULL) ? "Yes" : "NO! Error!");
    //abort();
    assert(not_found == 0); // Core functionality check: if we put it, we should find it.
    assert(verified_count == insertion_attempts); // All attempted keys should be findable.
    assert(stress_map.has_null_key && IntMap_get(&stress_map, NULL) != NULL); // Check NULL persistence

    printf("Freeing stress test map...\n");
    IntMap_free(&stress_map);
    free(keys_inserted);
    printf("Stress test map freed.\n");
    printf("--- IntMap Stress Test Finished ---\n");
}


void run_ptrset_stress_test(int num_elements) {
    printf("\n--- Running PtrSet Stress Test (%d non-NULL elements) ---\n", num_elements);

    PtrSet stress_set;
    PtrSet_init(&stress_set);

    // Test adding NULL before stress
    printf("Adding NULL element before stress...\n");
    assert(PtrSet_add(&stress_set, NULL));
    assert(stress_set.has_null_element);
    assert(stress_set.current_size == 0);
    assert(PtrSet_contains(&stress_set, NULL));
    assert(PtrSet_add(&stress_set, NULL) == false); // Add again fails


    void** keys_inserted = malloc(sizeof(void*) * num_elements);
    if (!keys_inserted) {
        fprintf(stderr, "Stress test memory allocation failed.\n");
        PtrSet_free(&stress_set);
        return;
    }

    srand((unsigned int)time(NULL) ^ 0xDEADBEEF); // Re-seed differently

    printf("Adding %d non-NULL elements...\n", num_elements);
    int addition_attempts = 0;
    int successful_adds = 0; // Count unique non-NULL additions

    for (int i = 0; i < num_elements; ++i) {
        void* key = generate_random_key(); // Ensures non-NULL
        keys_inserted[addition_attempts] = key;
        addition_attempts++;

        if (PtrSet_add(&stress_set, key)) {
            successful_adds++;
        }

        // Check NULL element is still present
        assert(stress_set.has_null_element);
        assert(PtrSet_contains(&stress_set, NULL));
        // Verify size reflects only non-NULL elements
        assert(stress_set.current_size == (uint64_t)successful_adds);

        if (num_elements <= 10000 || (i + 1) % (num_elements / 20) == 0) {
             bool is_inline = (stress_set.table == stress_set.inline_buffer);
             uint64_t capacity = (stress_set.table == stress_set.inline_buffer) ? PHM_INLINE_CAPACITY : stress_set.current_capacity;
             printf("... attempted %d non-NULL adds (set non-NULL size: %llu, capacity: %llu, %s, NULL present: %d)\n",
                    i + 1, (unsigned long long)stress_set.current_size,
                    (unsigned long long)capacity, (stress_set.table == stress_set.inline_buffer) ? "inline" : "heap", stress_set.has_null_element);
        }
    }

    uint64_t final_non_null_size = stress_set.current_size;
    printf("Addition complete. Attempted non-NULL additions: %d. Successful unique non-NULL adds: %d. Final non-NULL set size: %llu. NULL element present: %d\n",
           addition_attempts, successful_adds, (unsigned long long)final_non_null_size, stress_set.has_null_element);
    assert(final_non_null_size == (uint64_t)successful_adds); // Size should match unique non-NULL additions

    printf("Verifying %d potentially added non-NULL keys...\n", addition_attempts);
    int verified_count = 0;
    int not_found = 0;

    // Check NULL first
    assert(stress_set.has_null_element);
    assert(PtrSet_contains(&stress_set, NULL));

    // Verify presence of all *attempted* non-NULL keys
    for (int i = 0; i < addition_attempts; ++i) {
        void* key_to_check = keys_inserted[i];
        assert(key_to_check != NULL);

        if (PtrSet_contains(&stress_set, key_to_check)) {
            verified_count++;
        } else {
             // This *could* happen if PtrSet_add returned true but failed internally,
             // or if the key wasn't actually successfully added (due to duplicates).
             // Let's re-check if it *should* be present.
             bool should_be_present = false;
             for(int j=0; j<=i; ++j) { // Check if this key was the *last* add attempt for this value
                 if (keys_inserted[j] == key_to_check) {
                     // We need a way to know if *any* add succeeded for this key.
                     // The simple check is just that `contains` should work if `add`
                     // was ever called for it. A robust test might need to track
                     // successful adds per key, but let's stick to basic check.
                     should_be_present = true; // Assume it should be present if attempted
                 }
             }
             // If we attempted to add it, it should be contained, even if subsequent adds failed.
             // A failure here likely indicates contains() is broken or add() had critical failure.
             fprintf(stderr, "ERROR: Non-NULL Key %p (attempt index %d) was added but not found by contains()!\n", key_to_check, i);
             not_found++;
        }
         if (num_elements <= 10000 || (i + 1) % (addition_attempts / 10) == 0 || addition_attempts < 20) {
            printf("... verified presence for non-NULL attempt %d\n", i + 1);
        }
    }

    printf("Verification complete.\n");
    printf("Total attempted non-NULL additions: %d\n", addition_attempts);
    printf("Final non-NULL set size (unique elements): %llu\n", (unsigned long long)final_non_null_size);
    printf("Non-NULL Keys successfully found by contains(): %d\n", verified_count);
    printf("Non-NULL Keys not found by contains(): %d\n", not_found);
    printf("Has null element: %s\n", stress_set.has_null_element ? "true" : "false");
    PtrSet_add(&stress_set, NULL);
    printf("Has null element now: %s\n", stress_set.has_null_element ? "true" : "false");
    printf("NULL element present and verified: %s\n", (stress_set.has_null_element && PtrSet_contains(&stress_set, NULL)) ? "Yes" : "NO! Error!");


    assert(not_found == 0);
    // Every key we attempted to add should be present in the set afterwards.
    assert(verified_count == addition_attempts);
    assert(stress_set.has_null_element && PtrSet_contains(&stress_set, NULL)); // Check NULL persistence


    printf("Freeing stress test set...\n");
    PtrSet_free(&stress_set);
    free(keys_inserted);
    printf("Stress test set freed.\n");
    printf("--- PtrSet Stress Test Finished ---\n");
}


// --- Main Test Function ---
int main() {
    printf("PTR_HASH_MAP / PTR_HASH_SET Test Suite (with NULL key support)\n");
    printf("Inline Capacity: %d, Initial Heap Capacity: %d, Load Factor: %d/%d\n",
           PHM_INLINE_CAPACITY, PHM_INITIAL_HEAP_CAPACITY, PHM_LOAD_FACTOR_NUM, PHM_LOAD_FACTOR_DEN);

    // Define some keys (using integers cast to void* for simplicity in basic tests)
    void* key1 = (void*)0x100; // Use non-small integers to avoid confusion with NULL/0
    void* key2 = (void*)0x200;
    void* key3 = (void*)0x300;
    void* key4 = (void*)0x400; // Should trigger grow (assuming INLINE_CAPACITY is 3)
    void* key5 = (void*)0x500;
    void* key_nonexistent = (void*)0x990;
    void* null_key = NULL;

    int* val_ptr = NULL;
    float* fval_ptr = NULL;

    // ========================================================
    //                    IntMap Basic Tests
    // ========================================================
    printf("\n--- Testing IntMap ---\n");
    { // Scope for basic map tests
        IntMap imap;
        IntMap_init(&imap);
        printf("Initialized.\n");

        // Check initial state
        assert(imap.current_size == 0);
        assert(!imap.has_null_key);
        assert(imap.table == imap.inline_buffer);
        assert(IntMap_get(&imap, null_key) == NULL); // Should not find NULL key initially
        assert(IntMap_get(&imap, key1) == NULL);

        // --- NULL Key Handling ---
        printf("Putting NULL key -> 999 (inline)\n");
        IntMap_put(&imap, null_key, 999);
        assert(imap.current_size == 0);    // Size should *not* change for NULL key
        assert(imap.has_null_key);       // Flag should be set
        val_ptr = IntMap_get(&imap, null_key);
        assert(val_ptr != NULL && *val_ptr == 999); // Should find NULL key now
        assert(val_ptr == &imap.null_value);     // Verify it points to the dedicated slot

        // --- Inline Operations (with NULL already present) ---
        printf("Putting key1 -> 10 (inline, NULL present)\n");
        IntMap_put(&imap, key1, 10);
        assert(imap.current_size == 1); // Non-NULL size increments
        assert(imap.has_null_key);     // NULL still present
        assert(imap.table == imap.inline_buffer);
        val_ptr = IntMap_get(&imap, key1);
        assert(val_ptr != NULL && *val_ptr == 10);
        val_ptr = IntMap_get(&imap, null_key);   // Verify NULL still accessible
        assert(val_ptr != NULL && *val_ptr == 999);
        assert(IntMap_get(&imap, key2) == NULL); // Check non-existent

        printf("Putting key2 -> 20 (inline)\n");
        IntMap_put(&imap, key2, 20);
        assert(imap.current_size == 2);

        printf("Putting key3 -> 30 (inline)\n");
        IntMap_put(&imap, key3, 30);
        assert(imap.current_size == 3);
        assert(imap.has_null_key);
        assert(imap.table == imap.inline_buffer); // Still inline (at capacity if 3)

        printf("Updating NULL key -> 888 (inline)\n");
        IntMap_put(&imap, null_key, 888);
        assert(imap.current_size == 3);    // Size doesn't change
        assert(imap.has_null_key);
        val_ptr = IntMap_get(&imap, null_key);
        assert(val_ptr != NULL && *val_ptr == 888); // Value updated

        printf("Updating key2 -> 25 (inline)\n");
        IntMap_put(&imap, key2, 25);
        assert(imap.current_size == 3); // Non-NULL Size doesn't change on update
        val_ptr = IntMap_get(&imap, key2);
        assert(val_ptr != NULL && *val_ptr == 25);

        // --- Grow (with NULL key present) ---
        printf("Putting key4 -> 40 (triggers grow to heap, NULL present)\n");
        IntMap_put(&imap, key4, 40);
        assert(imap.current_size == 4); // Non-NULL size increments
        assert(imap.has_null_key);     // NULL should persist after grow
        assert(imap.table != imap.inline_buffer); // Should be on heap now
        assert(imap.current_capacity >= PHM_INITIAL_HEAP_CAPACITY); // Check capacity

        // Verify all elements after grow
        printf("Verifying elements after grow...\n");
        val_ptr = IntMap_get(&imap, null_key); assert(val_ptr != NULL && *val_ptr == 888); // Updated NULL value
        val_ptr = IntMap_get(&imap, key1); assert(val_ptr != NULL && *val_ptr == 10);
        val_ptr = IntMap_get(&imap, key2); assert(val_ptr != NULL && *val_ptr == 25); // Updated value
        val_ptr = IntMap_get(&imap, key3); assert(val_ptr != NULL && *val_ptr == 30);
        val_ptr = IntMap_get(&imap, key4); assert(val_ptr != NULL && *val_ptr == 40);
        assert(IntMap_get(&imap, key5) == NULL); // Not added yet

        // --- Heap Operations (with NULL present) ---
        printf("Putting key5 -> 50 (heap)\n");
        IntMap_put(&imap, key5, 50);
        assert(imap.current_size == 5); // Non-NULL count
        assert(imap.has_null_key);
        val_ptr = IntMap_get(&imap, key5);
        assert(val_ptr != NULL && *val_ptr == 50);

        printf("Updating NULL key -> 777 (heap)\n");
        IntMap_put(&imap, null_key, 777);
        assert(imap.current_size == 5);    // Size doesn't change
        assert(imap.has_null_key);
        val_ptr = IntMap_get(&imap, null_key);
        assert(val_ptr != NULL && *val_ptr == 777); // Value updated

        printf("Getting non-existent key 990\n");
        val_ptr = IntMap_get(&imap, key_nonexistent);
        assert(val_ptr == NULL);

        // --- _for_each Test ---
        printf("Testing _for_each...\n");
        IntMap imap_fe;
        IntMap_init(&imap_fe);
        IntMap_put(&imap_fe, key1, 11);
        IntMap_put(&imap_fe, null_key, 55);
        IntMap_put(&imap_fe, key2, 22);

        IntMapForEachData fe_data = {0};
        IntMap_for_each(&imap_fe, intmap_for_each_callback, &fe_data);

        assert(fe_data.count == 3); // NULL + key1 + key2
        assert(fe_data.found_null);
        assert(fe_data.found_key1);
        assert(fe_data.found_key2);
        assert(fe_data.null_value == 55);
        assert(fe_data.key1_value == 11);
        assert(fe_data.key2_value == 22);
        IntMap_free(&imap_fe);
        printf("_for_each test passed.\n");


        // --- Free (with NULL present) ---
        printf("Freeing IntMap\n");
        IntMap_free(&imap); // imap had NULL key and key1-5
        assert(imap.current_size == 0);
        assert(!imap.has_null_key); // Should reset NULL flag
        assert(imap.table == imap.inline_buffer); // Should reset to inline
        // Verify it's truly empty
        assert(IntMap_get(&imap, null_key) == NULL);
        assert(IntMap_get(&imap, key1) == NULL);
        assert(IntMap_get(&imap, key4) == NULL);

        // Test reuse after free
        printf("Putting key1 -> 100 after free\n");
        IntMap_put(&imap, key1, 100);
        assert(imap.current_size == 1);
        assert(!imap.has_null_key);
        val_ptr = IntMap_get(&imap, key1);
        assert(val_ptr != NULL && *val_ptr == 100);
        assert(IntMap_get(&imap, null_key) == NULL);

        printf("Putting NULL -> 666 after free and reuse\n");
        IntMap_put(&imap, null_key, 666);
        assert(imap.current_size == 1); // non-NULL size unchanged
        assert(imap.has_null_key);
        val_ptr = IntMap_get(&imap, null_key);
        assert(val_ptr != NULL && *val_ptr == 666);
        val_ptr = IntMap_get(&imap, key1); // check key1 still there
        assert(val_ptr != NULL && *val_ptr == 100);

        IntMap_free(&imap); // Free again

        printf("IntMap basic tests passed.\n");
    } // End scope for basic map tests


    // ========================================================
    //                FloatMap Basic Tests
    // ========================================================
     printf("\n--- Testing FloatMap ---\n");
    { // Scope for float map tests
        FloatMap fmap;
        FloatMap_init(&fmap);
        printf("Initialized.\n");

        assert(fmap.current_size == 0);
        assert(!fmap.has_null_key);
        assert(fmap.table == fmap.inline_buffer);
        assert(FloatMap_get(&fmap, null_key) == NULL);

        // --- NULL Key Handling ---
        printf("Putting NULL key -> 9.9f (inline)\n");
        FloatMap_put(&fmap, null_key, 9.9f);
        assert(fmap.current_size == 0);
        assert(fmap.has_null_key);
        fval_ptr = FloatMap_get(&fmap, null_key);
        assert(fval_ptr != NULL && float_eq(*fval_ptr, 9.9f));
        assert(fval_ptr == &fmap.null_value);

        // Use same keys, different values
        float fval1 = 1.1f;
        float fval2 = 2.2f;
        float fval3 = 3.3f;
        float fval4 = 4.4f; // Trigger grow

        // --- Inline (with NULL) ---
        printf("Putting key 1 -> 1.1 (inline, NULL present)\n");
        FloatMap_put(&fmap, key1, fval1);
        assert(fmap.current_size == 1);
        assert(fmap.has_null_key);
        fval_ptr = FloatMap_get(&fmap, key1);
        assert(fval_ptr != NULL && float_eq(*fval_ptr, fval1));
        fval_ptr = FloatMap_get(&fmap, null_key);
        assert(fval_ptr != NULL && float_eq(*fval_ptr, 9.9f));


        printf("Putting key 2 -> 2.2 (inline)\n"); FloatMap_put(&fmap, key2, fval2);
        printf("Putting key 3 -> 3.3 (inline)\n"); FloatMap_put(&fmap, key3, fval3);
        assert(fmap.current_size == 3);
        assert(fmap.table == fmap.inline_buffer);

        printf("Updating NULL key -> 8.8f (inline)\n");
        FloatMap_put(&fmap, null_key, 8.8f);
        assert(fmap.current_size == 3);
        assert(fmap.has_null_key);
        fval_ptr = FloatMap_get(&fmap, null_key);
        assert(fval_ptr != NULL && float_eq(*fval_ptr, 8.8f));


        // --- Grow (with NULL)---
        printf("Putting key 4 -> 4.4 (triggers grow to heap, NULL present)\n");
        FloatMap_put(&fmap, key4, fval4);
        assert(fmap.current_size == 4);
        assert(fmap.has_null_key);
        assert(fmap.table != fmap.inline_buffer);
        assert(fmap.current_capacity >= PHM_INITIAL_HEAP_CAPACITY);

        // Verify elements after grow
        fval_ptr = FloatMap_get(&fmap, null_key); assert(fval_ptr != NULL && float_eq(*fval_ptr, 8.8f)); // Updated NULL
        fval_ptr = FloatMap_get(&fmap, key1); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval1));
        fval_ptr = FloatMap_get(&fmap, key2); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval2));
        fval_ptr = FloatMap_get(&fmap, key3); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval3));
        fval_ptr = FloatMap_get(&fmap, key4); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval4));

        // --- Heap (with NULL)---
        printf("Updating NULL key -> 7.7f (heap)\n");
        FloatMap_put(&fmap, null_key, 7.7f);
        assert(fmap.current_size == 4); // non-NULL size unchanged
        fval_ptr = FloatMap_get(&fmap, null_key);
        assert(fval_ptr != NULL && float_eq(*fval_ptr, 7.7f));


        // --- Free ---
        printf("Freeing FloatMap\n");
        FloatMap_free(&fmap);
        assert(fmap.current_size == 0);
        assert(!fmap.has_null_key);
        assert(fmap.table == fmap.inline_buffer);
        assert(FloatMap_get(&fmap, key1) == NULL);
        assert(FloatMap_get(&fmap, null_key) == NULL);

        printf("FloatMap basic tests passed.\n");
    } // End scope for float map tests


    // ========================================================
    //                    PtrSet Basic Tests
    // ========================================================
    printf("\n--- Testing PtrSet ---\n");
    { // Scope for basic set tests
        PtrSet pset;
        PtrSet_init(&pset);
        printf("Initialized.\n");

        // Check initial state
        assert(pset.current_size == 0);
        assert(!pset.has_null_element);
        assert(pset.table == pset.inline_buffer);
        assert(PtrSet_contains(&pset, null_key) == false);
        assert(PtrSet_contains(&pset, key1) == false);

        // --- NULL Element Handling ---
        printf("Adding NULL element (inline)\n");
        assert(PtrSet_add(&pset, null_key) == true); // First add succeeds
        assert(pset.current_size == 0);             // Size should *not* change for NULL
        assert(pset.has_null_element);             // Flag should be set
        assert(PtrSet_contains(&pset, null_key) == true); // Should contain NULL now

        printf("Adding NULL element again (inline, should fail)\n");
        assert(PtrSet_add(&pset, null_key) == false); // Second add fails
        assert(pset.current_size == 0);              // Size still 0
        assert(pset.has_null_element);              // Flag still set
        assert(PtrSet_contains(&pset, null_key) == true);

        // --- Inline Operations (with NULL already present) ---
        printf("Adding key1 (inline, NULL present)\n");
        assert(PtrSet_add(&pset, key1) == true);
        assert(pset.current_size == 1); // Non-NULL size increments
        assert(pset.has_null_element);  // NULL still present
        assert(pset.table == pset.inline_buffer);
        assert(PtrSet_contains(&pset, key1) == true);
        assert(PtrSet_contains(&pset, null_key) == true); // Verify NULL still contained
        assert(PtrSet_contains(&pset, key2) == false);   // Check non-existent

        printf("Adding key2 (inline)\n"); assert(PtrSet_add(&pset, key2) == true); assert(pset.current_size == 2);
        printf("Adding key3 (inline)\n"); assert(PtrSet_add(&pset, key3) == true); assert(pset.current_size == 3);
        assert(pset.has_null_element);
        assert(pset.table == pset.inline_buffer); // Still inline (at capacity if 3)

        printf("Adding key2 again (inline, should fail)\n");
        assert(PtrSet_add(&pset, key2) == false); // Already exists
        assert(pset.current_size == 3); // Non-NULL Size doesn't change
        assert(PtrSet_contains(&pset, key2) == true);

        // --- Grow (with NULL element present) ---
        printf("Adding key4 (triggers grow to heap, NULL present)\n");
        assert(PtrSet_add(&pset, key4) == true);
        assert(pset.current_size == 4); // Non-NULL size increments
        assert(pset.has_null_element); // NULL should persist after grow
        assert(pset.table != pset.inline_buffer); // Should be on heap now
        assert(pset.current_capacity >= PHM_INITIAL_HEAP_CAPACITY); // Check capacity

        // Verify all elements after grow
        printf("Verifying elements after grow...\n");
        assert(PtrSet_contains(&pset, null_key) == true); // NULL present
        assert(PtrSet_contains(&pset, key1) == true);
        assert(PtrSet_contains(&pset, key2) == true);
        assert(PtrSet_contains(&pset, key3) == true);
        assert(PtrSet_contains(&pset, key4) == true);
        assert(PtrSet_contains(&pset, key5) == false); // Not added yet

        // --- Heap Operations (with NULL present) ---
        printf("Adding key5 (heap)\n");
        assert(PtrSet_add(&pset, key5) == true);
        assert(pset.current_size == 5); // Non-NULL count
        assert(pset.has_null_element);
        assert(PtrSet_contains(&pset, key5) == true);

        printf("Adding NULL element again (heap, should fail)\n");
        assert(PtrSet_add(&pset, null_key) == false);
        assert(pset.current_size == 5); // Size unchanged
        assert(PtrSet_contains(&pset, null_key) == true);

        printf("Adding key1 again (heap, should fail)\n");
        assert(PtrSet_add(&pset, key1) == false);
        assert(pset.current_size == 5);
        assert(PtrSet_contains(&pset, key1) == true);

        printf("Checking non-existent key 990\n");
        assert(PtrSet_contains(&pset, key_nonexistent) == false);

        // --- _for_each Test ---
        printf("Testing _for_each...\n");
        PtrSet pset_fe;
        PtrSet_init(&pset_fe);
        assert(PtrSet_add(&pset_fe, key1));
        assert(PtrSet_add(&pset_fe, null_key));
        assert(PtrSet_add(&pset_fe, key2));
        assert(PtrSet_add(&pset_fe, key1) == false); // Add duplicate non-NULL
        assert(PtrSet_add(&pset_fe, null_key) == false); // Add duplicate NULL

        PtrSetForEachData fe_data = {0};
        PtrSet_for_each(&pset_fe, ptrset_for_each_callback, &fe_data);

        assert(fe_data.count == 3); // NULL + key1 + key2
        assert(fe_data.found_null);
        assert(fe_data.found_key1);
        assert(fe_data.found_key2);
        PtrSet_free(&pset_fe);
        printf("_for_each test passed.\n");

        // --- Free (with NULL present) ---
        printf("Freeing PtrSet\n");
        PtrSet_free(&pset); // pset had NULL and key1-5
        assert(pset.current_size == 0);
        assert(!pset.has_null_element); // Should reset NULL flag
        assert(pset.table == pset.inline_buffer); // Should reset to inline
        // Verify it's truly empty
        assert(PtrSet_contains(&pset, null_key) == false);
        assert(PtrSet_contains(&pset, key1) == false);
        assert(PtrSet_contains(&pset, key4) == false);

        // Test reuse after free
        printf("Adding key1 after free\n");
        assert(PtrSet_add(&pset, key1) == true);
        assert(pset.current_size == 1);
        assert(!pset.has_null_element);
        assert(PtrSet_contains(&pset, key1) == true);
        assert(PtrSet_contains(&pset, null_key) == false);

        printf("Adding NULL after free and reuse\n");
        assert(PtrSet_add(&pset, null_key) == true);
        assert(pset.current_size == 1); // non-NULL size unchanged
        assert(pset.has_null_element);
        assert(PtrSet_contains(&pset, null_key) == true);
        assert(PtrSet_contains(&pset, key1) == true); // check key1 still there

        PtrSet_free(&pset); // Free again

        printf("PtrSet basic tests passed.\n");
    } // End scope for basic set tests


    // ========================================================
    //                    STRESS TEST CALLS
    // ========================================================
    // Use smaller numbers for quick tests, larger for thoroughness
    // int stress_elements = 10000; // Quick run
    int stress_elements = 200000; // Moderate run (~0.2 million)
    // int stress_elements = 1000000; // Longer run (1 million)

    run_intmap_stress_test(stress_elements);
    run_ptrset_stress_test(stress_elements);


    printf("\nAll tests finished.\n");
    return 0;
}