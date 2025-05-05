#include <stdio.h>
#include <assert.h>
#include <math.h> // For fabsf
#include <stdbool.h>
#include <stdlib.h> // For rand, srand, malloc, free
#include <time.h>   // For time (seeding rand)
#include <stdint.h> // For uintptr_t

// Define ptr_hash_map.h / ptr_hash_set.h to be the generic header file name
// Assuming you saved the combined code as "ptr_hash_tables_generic.h"
#include <ptr_hash_map.h>

// Or include them separately if you kept them in separate files:
// #include <ptr_hash_map.h>
// #include <ptr_hash_set.h>

// Use external cc library (optional, remove if not using)
#define CC_NO_SHORT_NAMES
#include <external/cc.h>


// --- Define Specific Types ---
DEFINE_PTR_HASH_MAP(static inline, IntMap, int)
DEFINE_PTR_HASH_MAP(static inline, FloatMap, float)
DEFINE_PTR_HASH_SET(static inline, PtrSet) // Define the pointer set type

// --- Helpers ---
static inline bool float_eq(float a, float b) {
    return fabsf(a - b) < 1e-6f;
}

// Generate a non-NULL pseudo-random pointer key
static inline void* generate_random_key() {
    uintptr_t random_key_val = ((uintptr_t)rand() << 31) ^ ((uintptr_t)rand()); // Wider range
    // Ensure key is not NULL (which is disallowed)
    if (random_key_val == 0) random_key_val = 1;
    // Offset from NULL to create a plausible pointer address
    return (void*)random_key_val;
}

// --- Stress Tests ---

void run_intmap_stress_test(int num_elements) {
    printf("\n--- Running IntMap Stress Test (%d elements) ---\n", num_elements);

    IntMap stress_map;
    
    IntMap_init(&stress_map);

    // Store pointers to keys actually used, not just their integer representation
    void** keys_inserted = malloc(sizeof(void*) * num_elements);
    int* expected_values = malloc(sizeof(int) * num_elements); // Track expected value *at insertion time*
    if (!keys_inserted || !expected_values) {
        fprintf(stderr, "Stress test memory allocation failed.\n");
        free(keys_inserted);
        free(expected_values);
        IntMap_free(&stress_map);
        return;
    }

    srand((unsigned int)time(NULL)); // Seed random number generator

    printf("Inserting %d elements...\n", num_elements);
    int insertion_attempts = 0;
    for (int i = 0; i < num_elements; ++i) {
        void* key = generate_random_key();
        int value = i + 1; // Assign a unique value based on loop index

        keys_inserted[insertion_attempts] = key;
        expected_values[insertion_attempts] = value; // Store expected value for this attempt
        insertion_attempts++;

        IntMap_put(&stress_map, key, value);

        // Print progress (less frequently for large numbers)
        if (num_elements <= 10000 || (i + 1) % (num_elements / 20) == 0) {
             bool is_inline = (stress_map.table == stress_map.inline_buffer);
             uint64_t capacity = is_inline ? PHM_INLINE_CAPACITY : stress_map.current_capacity;
             printf("... attempted %d inserts (map size: %llu, capacity: %llu, %s)\n",
                    i + 1, (unsigned long long)stress_map.current_size,
                    (unsigned long long)capacity, is_inline ? "inline" : "heap");
        }
    }

    uint64_t final_size = stress_map.current_size;
    printf("Insertion complete. Attempted insertions: %d. Final unique map size: %llu\n",
           insertion_attempts, (unsigned long long)final_size);
    // Note: final_size might be less than num_elements if rand() produced duplicate keys.

    printf("Verifying %d inserted key-value pairs...\n", insertion_attempts);
    int verified_count = 0;
    int not_found = 0;
    int wrong_value = 0;

    // We iterate through the keys we *attempted* to insert.
    // The value found should correspond to the *last* value inserted for that specific key.
    // This verification is tricky with duplicates. A simpler check is just presence.
    for (int i = 0; i < insertion_attempts; ++i) {
        void* key_to_check = keys_inserted[i];
        // int last_expected_value_for_this_key = ??? // Hard to track easily without another map

        int* found_value_ptr = IntMap_get(&stress_map, key_to_check);

        if (found_value_ptr != NULL) {
            verified_count++;
            // We cannot reliably check the value against expected_values[i]
            // because a later insertion with the same key might have overwritten it.
            // Verifying presence is the most robust check here without extra tracking.
        } else {
            // This should ideally not happen.
            fprintf(stderr, "ERROR: Key %p (attempt index %d) was put but not found!\n", key_to_check, i);
            not_found++;
        }

        // Print progress
        if (num_elements <= 10000 || (i + 1) % (insertion_attempts / 10) == 0 || insertion_attempts < 20) {
            printf("... verified presence for attempt %d\n", i + 1);
        }
    }

    printf("Verification complete.\n");
    printf("Total attempted insertions: %d\n", insertion_attempts);
    printf("Final map size (unique keys): %llu\n", (unsigned long long)final_size);
    printf("Keys successfully found during verification: %d\n", verified_count);
    printf("Keys not found during verification: %d\n", not_found);

    assert(not_found == 0); // Core functionality check: if we put it, we should find it.
    // We expect verified_count == insertion_attempts if everything worked.
    assert(verified_count == insertion_attempts);


    printf("Freeing stress test map...\n");
    IntMap_free(&stress_map);
    free(keys_inserted);
    free(expected_values);
    printf("Stress test map freed.\n");
    printf("--- IntMap Stress Test Finished ---\n");
}


void run_ptrset_stress_test(int num_elements) {
    printf("\n--- Running PtrSet Stress Test (%d elements) ---\n", num_elements);

    PtrSet stress_set;
    PtrSet_init(&stress_set);

    void** keys_inserted = malloc(sizeof(void*) * num_elements);
    if (!keys_inserted) {
        fprintf(stderr, "Stress test memory allocation failed.\n");
        PtrSet_free(&stress_set);
        return;
    }

    srand((unsigned int)time(NULL)); // Re-seed or use different seed? Using same for simplicity.

    printf("Adding %d elements...\n", num_elements);
    int addition_attempts = 0;
    int successful_adds = 0; // Count unique additions
    for (int i = 0; i < num_elements; ++i) {
        void* key = generate_random_key();

        keys_inserted[addition_attempts] = key;
        addition_attempts++;

        if (PtrSet_add(&stress_set, key)) {
            successful_adds++;
        }

        if (num_elements <= 10000 || (i + 1) % (num_elements / 20) == 0) {
             bool is_inline = (stress_set.table == stress_set.inline_buffer);
             uint64_t capacity = is_inline ? PHM_INLINE_CAPACITY : stress_set.current_capacity;
             printf("... attempted %d adds (set size: %llu, capacity: %llu, %s)\n",
                    i + 1, (unsigned long long)stress_set.current_size,
                    (unsigned long long)capacity, is_inline ? "inline" : "heap");
        }
    }

    uint64_t final_size = stress_set.current_size;
    printf("Addition complete. Attempted additions: %d. Successful unique adds: %d. Final set size: %llu\n",
           addition_attempts, successful_adds, (unsigned long long)final_size);
    assert(final_size == (uint64_t)successful_adds); // Size should match unique additions

    printf("Verifying %d potentially added keys...\n", addition_attempts);
    int verified_count = 0;
    int not_found = 0;

    for (int i = 0; i < addition_attempts; ++i) {
        void* key_to_check = keys_inserted[i];

        if (PtrSet_contains(&stress_set, key_to_check)) {
            verified_count++;
        } else {
            // This should not happen for keys that were successfully added.
            // It *could* happen if PtrSet_add failed silently (e.g., grow failed).
            fprintf(stderr, "ERROR: Key %p (attempt index %d) was added but not found!\n", key_to_check, i);
            not_found++;
        }
         if (num_elements <= 10000 || (i + 1) % (addition_attempts / 10) == 0 || addition_attempts < 20) {
            printf("... verified presence for attempt %d\n", i + 1);
        }
    }

    printf("Verification complete.\n");
    printf("Total attempted additions: %d\n", addition_attempts);
    printf("Final set size (unique keys): %llu\n", (unsigned long long)final_size);
    printf("Keys successfully found during verification: %d\n", verified_count);
    printf("Keys not found during verification: %d\n", not_found);

    assert(not_found == 0);
    // Every key we attempted to add should be present in the set afterwards.
    assert(verified_count == addition_attempts);

    printf("Freeing stress test set...\n");
    PtrSet_free(&stress_set);
    free(keys_inserted);
    printf("Stress test set freed.\n");
    printf("--- PtrSet Stress Test Finished ---\n");
}


// --- Main Test Function ---
int main() {
    printf("PTR_HASH_MAP / PTR_HASH_SET Test Suite\n");
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

    // ========================================================
    //                    IntMap Basic Tests
    // ========================================================
    printf("\n--- Testing IntMap ---\n");
    { // Scope for basic map tests
        IntMap imap;
        IntMap_init(&imap);
        printf("Initialized.\n");

        // Check initial state & NULL key handling
        assert(imap.current_size == 0);
        assert(imap.table == imap.inline_buffer);
        printf("Checking NULL key handling...\n");
        IntMap_put(&imap, null_key, 999); // Should do nothing
        assert(imap.current_size == 0);    // Size should remain 0
        assert(IntMap_get(&imap, null_key) == NULL); // Should not find NULL key

        // --- Inline Operations ---
        printf("Putting key1 -> 10 (inline)\n");
        IntMap_put(&imap, key1, 10);
        assert(imap.current_size == 1);
        assert(imap.table == imap.inline_buffer);
        int* val_ptr = IntMap_get(&imap, key1);
        assert(val_ptr != NULL && *val_ptr == 10);
        assert(IntMap_get(&imap, key2) == NULL); // Check non-existent

        printf("Putting key2 -> 20 (inline)\n");
        IntMap_put(&imap, key2, 20);
        assert(imap.current_size == 2);
        val_ptr = IntMap_get(&imap, key2);
        assert(val_ptr != NULL && *val_ptr == 20);

        printf("Putting key3 -> 30 (inline)\n");
        IntMap_put(&imap, key3, 30);
        assert(imap.current_size == 3);
        assert(imap.table == imap.inline_buffer); // Still inline (at capacity if 3)
        val_ptr = IntMap_get(&imap, key3);
        assert(val_ptr != NULL && *val_ptr == 30);

        printf("Updating key2 -> 25 (inline)\n");
        IntMap_put(&imap, key2, 25);
        assert(imap.current_size == 3); // Size doesn't change on update
        val_ptr = IntMap_get(&imap, key2);
        assert(val_ptr != NULL && *val_ptr == 25);

        printf("Putting key1 again -> 15 (update inline)\n");
        IntMap_put(&imap, key1, 15);
        assert(imap.current_size == 3);
        val_ptr = IntMap_get(&imap, key1);
        assert(val_ptr != NULL && *val_ptr == 15);


        // --- Grow ---
        printf("Putting key4 -> 40 (triggers grow to heap)\n");
        IntMap_put(&imap, key4, 40);
        assert(imap.current_size == 4);
        assert(imap.table != imap.inline_buffer); // Should be on heap now
        assert(imap.current_capacity >= PHM_INITIAL_HEAP_CAPACITY); // Check capacity

        // Verify all elements after grow
        printf("Verifying elements after grow...\n");
        val_ptr = IntMap_get(&imap, key1); assert(val_ptr != NULL && *val_ptr == 15); // Updated value
        val_ptr = IntMap_get(&imap, key2); assert(val_ptr != NULL && *val_ptr == 25); // Updated value
        val_ptr = IntMap_get(&imap, key3); assert(val_ptr != NULL && *val_ptr == 30);
        val_ptr = IntMap_get(&imap, key4); assert(val_ptr != NULL && *val_ptr == 40);
        assert(IntMap_get(&imap, key5) == NULL); // Not added yet

        // --- Heap Operations ---
        printf("Putting key5 -> 50 (heap)\n");
        IntMap_put(&imap, key5, 50);
        assert(imap.current_size == 5);
        val_ptr = IntMap_get(&imap, key5);
        assert(val_ptr != NULL && *val_ptr == 50);

        printf("Updating key1 -> 11 (heap)\n");
        IntMap_put(&imap, key1, 11);
        assert(imap.current_size == 5);
        val_ptr = IntMap_get(&imap, key1);
        assert(val_ptr != NULL && *val_ptr == 11);

        printf("Getting non-existent key 990\n");
        val_ptr = IntMap_get(&imap, key_nonexistent);
        assert(val_ptr == NULL);

        printf("Modifying key3 via pointer\n");
        val_ptr = IntMap_get(&imap, key3);
        assert(val_ptr != NULL);
        if (val_ptr) { *val_ptr = 33; }
        val_ptr = IntMap_get(&imap, key3);
        assert(val_ptr != NULL && *val_ptr == 33);

        // --- Free ---
        printf("Freeing IntMap\n");
        IntMap_free(&imap);
        assert(imap.current_size == 0);
        assert(imap.table == imap.inline_buffer); // Should reset to inline
        // Verify it's truly empty
        assert(IntMap_get(&imap, key1) == NULL);
        assert(IntMap_get(&imap, key4) == NULL);
        // Check internal state if possible (optional, implementation detail)
        // assert(imap.inline_buffer[0].key == NULL); // Check first slot is clear

        // Test reuse after free
        printf("Putting key1 -> 100 after free\n");
        IntMap_put(&imap, key1, 100);
        assert(imap.current_size == 1);
        val_ptr = IntMap_get(&imap, key1);
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
        assert(fmap.table == fmap.inline_buffer);

        // NULL Key check
        printf("Checking NULL key handling...\n");
        FloatMap_put(&fmap, null_key, 9.9f);
        assert(fmap.current_size == 0);
        assert(FloatMap_get(&fmap, null_key) == NULL);

        // Use same keys, different values
        float fval1 = 1.1f;
        float fval2 = 2.2f;
        float fval3 = 3.3f;
        float fval4 = 4.4f; // Trigger grow

        // --- Inline ---
        printf("Putting key 1 -> 1.1 (inline)\n");
        FloatMap_put(&fmap, key1, fval1);
        assert(fmap.current_size == 1);
        float* fval_ptr = FloatMap_get(&fmap, key1);
        assert(fval_ptr != NULL && float_eq(*fval_ptr, fval1));

        printf("Putting key 2 -> 2.2 (inline)\n");
        FloatMap_put(&fmap, key2, fval2);
        printf("Putting key 3 -> 3.3 (inline)\n");
        FloatMap_put(&fmap, key3, fval3);
        assert(fmap.current_size == 3);
        assert(fmap.table == fmap.inline_buffer);

        printf("Updating key 2 -> 2.5 (inline)\n");
        FloatMap_put(&fmap, key2, 2.5f);
        assert(fmap.current_size == 3);
        fval_ptr = FloatMap_get(&fmap, key2);
        assert(fval_ptr != NULL && float_eq(*fval_ptr, 2.5f));

        // --- Grow ---
        printf("Putting key 4 -> 4.4 (triggers grow to heap)\n");
        FloatMap_put(&fmap, key4, fval4);
        assert(fmap.current_size == 4);
        assert(fmap.table != fmap.inline_buffer);
        assert(fmap.current_capacity >= PHM_INITIAL_HEAP_CAPACITY);

        // Verify elements after grow
        fval_ptr = FloatMap_get(&fmap, key1); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval1));
        fval_ptr = FloatMap_get(&fmap, key2); assert(fval_ptr != NULL && float_eq(*fval_ptr, 2.5f)); // Updated
        fval_ptr = FloatMap_get(&fmap, key3); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval3));
        fval_ptr = FloatMap_get(&fmap, key4); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval4));

        // --- Heap ---
        printf("Getting non-existent key 990\n");
        fval_ptr = FloatMap_get(&fmap, key_nonexistent);
        assert(fval_ptr == NULL);

        printf("Modifying key 1 via pointer\n");
        fval_ptr = FloatMap_get(&fmap, key1);
        assert(fval_ptr != NULL);
        if (fval_ptr) { *fval_ptr = 1.5f; }
        fval_ptr = FloatMap_get(&fmap, key1);
        assert(fval_ptr != NULL && float_eq(*fval_ptr, 1.5f));

        // --- Free ---
        printf("Freeing FloatMap\n");
        FloatMap_free(&fmap);
        assert(fmap.current_size == 0);
        assert(fmap.table == fmap.inline_buffer);
        assert(FloatMap_get(&fmap, key1) == NULL);

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

        // Check initial state & NULL key handling
        assert(pset.current_size == 0);
        assert(pset.table == pset.inline_buffer);
        printf("Checking NULL key handling...\n");
        assert(PtrSet_add(&pset, null_key) == false); // Add should fail
        assert(pset.current_size == 0);             // Size should remain 0
        assert(PtrSet_contains(&pset, null_key) == false); // Should not contain NULL

        // --- Inline Operations ---
        printf("Adding key1 (inline)\n");
        assert(PtrSet_add(&pset, key1) == true);
        assert(pset.current_size == 1);
        assert(pset.table == pset.inline_buffer);
        assert(PtrSet_contains(&pset, key1) == true);
        assert(PtrSet_contains(&pset, key2) == false); // Check non-existent

        printf("Adding key2 (inline)\n");
        assert(PtrSet_add(&pset, key2) == true);
        assert(pset.current_size == 2);
        assert(PtrSet_contains(&pset, key2) == true);

        printf("Adding key3 (inline)\n");
        assert(PtrSet_add(&pset, key3) == true);
        assert(pset.current_size == 3);
        assert(pset.table == pset.inline_buffer); // Still inline (at capacity if 3)
        assert(PtrSet_contains(&pset, key3) == true);

        printf("Adding key2 again (inline, should fail)\n");
        assert(PtrSet_add(&pset, key2) == false); // Already exists
        assert(pset.current_size == 3); // Size doesn't change
        assert(PtrSet_contains(&pset, key2) == true);

        // --- Grow ---
        printf("Adding key4 (triggers grow to heap)\n");
        assert(PtrSet_add(&pset, key4) == true);
        assert(pset.current_size == 4);
        assert(pset.table != pset.inline_buffer); // Should be on heap now
        assert(pset.current_capacity >= PHM_INITIAL_HEAP_CAPACITY); // Check capacity

        // Verify all elements after grow
        printf("Verifying elements after grow...\n");
        assert(PtrSet_contains(&pset, key1) == true);
        assert(PtrSet_contains(&pset, key2) == true);
        assert(PtrSet_contains(&pset, key3) == true);
        assert(PtrSet_contains(&pset, key4) == true);
        assert(PtrSet_contains(&pset, key5) == false); // Not added yet

        // --- Heap Operations ---
        printf("Adding key5 (heap)\n");
        assert(PtrSet_add(&pset, key5) == true);
        assert(pset.current_size == 5);
        assert(PtrSet_contains(&pset, key5) == true);

        printf("Adding key1 again (heap, should fail)\n");
        assert(PtrSet_add(&pset, key1) == false);
        assert(pset.current_size == 5);
        assert(PtrSet_contains(&pset, key1) == true);

        printf("Checking non-existent key 990\n");
        assert(PtrSet_contains(&pset, key_nonexistent) == false);

        // --- Free ---
        printf("Freeing PtrSet\n");
        PtrSet_free(&pset);
        assert(pset.current_size == 0);
        assert(pset.table == pset.inline_buffer); // Should reset to inline
        // Verify it's truly empty
        assert(PtrSet_contains(&pset, key1) == false);
        assert(PtrSet_contains(&pset, key4) == false);

        // Test reuse after free
        printf("Adding key1 after free\n");
        assert(PtrSet_add(&pset, key1) == true);
        assert(pset.current_size == 1);
        assert(PtrSet_contains(&pset, key1) == true);
        PtrSet_free(&pset); // Free again

        printf("PtrSet basic tests passed.\n");
    } // End scope for basic set tests


    // ========================================================
    //                    STRESS TEST CALLS
    // ========================================================
    // Use smaller numbers for quick tests, larger for thoroughness
    // int stress_elements = 10000; // Quick run
    int stress_elements = 1000000; // Longer run (e.g., 1 million)

    run_intmap_stress_test(stress_elements);
    run_ptrset_stress_test(stress_elements);


    printf("\nAll tests finished.\n");
    return 0;
}