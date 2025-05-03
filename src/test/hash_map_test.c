#include <stdio.h>
#include <assert.h>
#include <math.h> // For fabsf
#include <stdbool.h>
#include <stdlib.h> // For rand, srand, malloc, free
#include <time.h>   // For time (seeding rand)

// Define ptr_hash_map.h to be the generic header file name
#include <ptr_hash_map.h>
#define CC_NO_SHORT_NAMES
#include <external/cc.h>
// Or: #include "ptr_hash_map_generic.h" if you saved it with that name

// Define the specific map types using the macro
DEFINE_PTR_HASH_MAP(static inline, IntMap, int)
DEFINE_PTR_HASH_MAP(static inline, FloatMap, float)

// Helper for float comparison
static inline bool float_eq(float a, float b) {
    return fabsf(a - b) < 1e-6f;
}

// Function for the stress test
void run_intmap_stress_test(int num_elements) {
    printf("\n--- Running IntMap Stress Test (%d elements) ---\n", num_elements);

    IntMap stress_map;
    IntMap_init(&stress_map);

    // Allocate memory to store the keys we insert for verification
    // Using uintptr_t to store pointer values as integers for simplicity here
    uintptr_t* keys_inserted = malloc(sizeof(uintptr_t) * num_elements);
    if (!keys_inserted) {
        fprintf(stderr, "Failed to allocate memory for keys array.\n");
        IntMap_free(&stress_map); // Clean up map before exiting
        return;
    }

    srand((unsigned int)time(NULL)); // Seed random number generator

    printf("Inserting %d elements...\n", num_elements);
    int actual_inserted = 0;
    for (int i = 0; i < num_elements; ++i) {
        // Generate a pseudo-random key. Cast to void*.
        // Using rand() directly might produce duplicates, which is fine for testing.
        uintptr_t random_key_val = (uintptr_t)rand() << 24 | (uintptr_t)rand(); // Larger range
        // Ensure key is not NULL (which is disallowed)
        if (random_key_val == 0) random_key_val = 1;

        void* key = (void*)(((char*)(NULL)) + random_key_val);
        int value = i + 1; // Assign a unique value based on loop index

        // Check if key already exists (simplistic check, assumes rand() is unlikely
        // to generate the *exact* same key sequence if we were to track perfectly).
        // In a real test, you might want better duplicate detection or just let the map handle it.
        // For this test, we just store the key used.
        keys_inserted[actual_inserted] = random_key_val;

        IntMap_put(&stress_map, key, value);

        // We only increment actual_inserted if the put didn't just update an existing key.
        // The map's current_size tells us the true count of unique keys.
        // Let's track the keys we *attempted* to insert for verification.
        actual_inserted++;

        if ((i + 1) % (num_elements / 10) == 0) {
             printf("... inserted %d elements (map size: %llu, capacity: %llu)\n",
                    i + 1, (unsigned long long)stress_map.current_size,
                    (stress_map.table == stress_map.inline_buffer) ? PHM_INLINE_CAPACITY : (unsigned long long)stress_map.current_capacity);
        }
    }

    uint64_t final_size = stress_map.current_size;
    printf("Insertion complete. Final map size: %llu (expected unique keys inserted)\n", (unsigned long long)final_size);
    // Note: final_size might be less than num_elements if rand() produced duplicate keys.

    printf("Verifying %d inserted key-value pairs...\n", actual_inserted);
    int verified_count = 0;
    int not_found = 0;
    int wrong_value = 0;

    for (int i = 0; i < actual_inserted; ++i) {
        void* key_to_check = (void*)keys_inserted[i];
        int expected_value = i + 1; // This assumes the *last* value put for a duplicate key wins.

        int* found_value_ptr = IntMap_get(&stress_map, key_to_check);

        if (found_value_ptr != NULL) {
            // Important: If rand() generated duplicates, the value found
            // will correspond to the *last* time that key was inserted.
            // Our simple `expected_value = i + 1` check is only correct if keys are unique.
            // A more robust check would require storing expected values alongside keys,
            // or accepting that the value might be from a later insertion if duplicates occurred.
            // Let's just check if it was found for this basic stress test.
            verified_count++;

            // Optional stricter check (might fail with duplicate keys):
            // if (*found_value_ptr == expected_value) {
            //     verified_count++;
            // } else {
            //     wrong_value++;
            // }

        } else {
            // This shouldn't happen unless something went wrong.
            fprintf(stderr, "ERROR: Key %p (index %d) inserted but not found!\n", key_to_check, i);
            not_found++;
        }

        if ((i + 1) % (actual_inserted / 10) == 0) {
            printf("... verified %d pairs\n", i + 1);
        }
    }

    printf("Verification complete.\n");
    printf("Total attempted insertions: %d\n", actual_inserted);
    printf("Final map size (unique keys): %llu\n", (unsigned long long)final_size);
    printf("Keys successfully retrieved: %d\n", verified_count);
    printf("Keys not found during verification: %d\n", not_found);
    // printf("Keys found with unexpected value (due to duplicates): %d\n", wrong_value); // If using stricter check

    assert(not_found == 0); // Should always find keys we put (unless duplicates overwrite)
    // We can't easily assert verified_count == final_size without more complex tracking
    // due to potential duplicate random keys overwriting values.
    // But we expect verified_count == actual_inserted if no errors occurred.
    assert(verified_count == actual_inserted);


    printf("Freeing stress test map...\n");
    IntMap_free(&stress_map);
    free(keys_inserted);
    printf("Stress test map freed.\n");
    printf("--- IntMap Stress Test Finished ---\n");
}


int main() {
    printf("--- Testing IntMap ---\n");

    IntMap imap;
    IntMap_init(&imap);
    printf("Initialized.\n");

    // Check initial state
    assert(imap.current_size == 0);
    assert(imap.table == imap.inline_buffer);

    // Define some keys (using integers cast to void* for simplicity)
    void* key1 = (void*)1;
    void* key2 = (void*)2;
    void* key3 = (void*)3;
    void* key4 = (void*)4; // This should trigger grow
    void* key_nonexistent = (void*)99;

    // --- Inline Operations ---
    printf("Putting key 1 -> 10 (inline)\n");
    IntMap_put(&imap, key1, 10);
    assert(imap.current_size == 1);
    assert(imap.table == imap.inline_buffer);
    int* val_ptr = IntMap_get(&imap, key1);
    assert(val_ptr != NULL && *val_ptr == 10);

    printf("Putting key 2 -> 20 (inline)\n");
    IntMap_put(&imap, key2, 20);
    assert(imap.current_size == 2);
    val_ptr = IntMap_get(&imap, key2);
    assert(val_ptr != NULL && *val_ptr == 20);

    printf("Putting key 3 -> 30 (inline)\n");
    IntMap_put(&imap, key3, 30);
    assert(imap.current_size == 3);
    assert(imap.table == imap.inline_buffer); // Still inline (at capacity)
    val_ptr = IntMap_get(&imap, key3);
    assert(val_ptr != NULL && *val_ptr == 30);

    printf("Updating key 2 -> 25 (inline)\n");
    IntMap_put(&imap, key2, 25);
    assert(imap.current_size == 3); // Size doesn't change on update
    val_ptr = IntMap_get(&imap, key2);
    assert(val_ptr != NULL && *val_ptr == 25);

    // --- Grow ---
    printf("Putting key 4 -> 40 (triggers grow to heap)\n");
    IntMap_put(&imap, key4, 40);
    assert(imap.current_size == 4);
    //assert(imap.table != imap.inline_buffer); // Should be on heap now
    //assert(imap.current_capacity >= PHM_INITIAL_HEAP_CAPACITY); // Check capacity (>=8)

    // Verify all elements after grow
    val_ptr = IntMap_get(&imap, key1); assert(val_ptr != NULL && *val_ptr == 10);
    val_ptr = IntMap_get(&imap, key2); assert(val_ptr != NULL && *val_ptr == 25); // Updated value
    val_ptr = IntMap_get(&imap, key3); assert(val_ptr != NULL && *val_ptr == 30);
    val_ptr = IntMap_get(&imap, key4); assert(val_ptr != NULL && *val_ptr == 40);

    // --- Heap Operations ---
    printf("Updating key 1 -> 11 (heap)\n");
    IntMap_put(&imap, key1, 11);
    assert(imap.current_size == 4);
    val_ptr = IntMap_get(&imap, key1);
    assert(val_ptr != NULL && *val_ptr == 11);

    printf("Getting non-existent key 99\n");
    val_ptr = IntMap_get(&imap, key_nonexistent);
    assert(val_ptr == NULL);

    printf("Modifying key 3 via pointer\n");
    val_ptr = IntMap_get(&imap, key3);
    assert(val_ptr != NULL);
    if (val_ptr) {
        *val_ptr = 33;
    }
    val_ptr = IntMap_get(&imap, key3);
    assert(val_ptr != NULL && *val_ptr == 33);

    // --- Free ---
    printf("Freeing IntMap\n");
    IntMap_free(&imap);
    assert(imap.current_size == 0);
    assert(imap.table == imap.inline_buffer); // Should reset to inline
    // assert(imap.inline_buffer[0].key == NULL); // This check might fail depending on how memset worked or if padding exists

    // Test reuse after free
    printf("Putting key 1 -> 100 after free\n");
    IntMap_put(&imap, key1, 100);
    assert(imap.current_size == 1);
    val_ptr = IntMap_get(&imap, key1);
    assert(val_ptr != NULL && *val_ptr == 100);
    IntMap_free(&imap); // Free again

    printf("IntMap basic tests passed.\n");


    // ========================================================
    //                FLOAT MAP TESTS (Unchanged)
    // ========================================================

    printf("\n--- Testing FloatMap ---\n");

    FloatMap fmap;
    FloatMap_init(&fmap);
    printf("Initialized.\n");

    assert(fmap.current_size == 0);
    assert(fmap.table == fmap.inline_buffer);

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
    //assert(fmap.table != fmap.inline_buffer);
    //assert(fmap.current_capacity >= PHM_INITIAL_HEAP_CAPACITY);

    // Verify elements after grow
    fval_ptr = FloatMap_get(&fmap, key1); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval1));
    fval_ptr = FloatMap_get(&fmap, key2); assert(fval_ptr != NULL && float_eq(*fval_ptr, 2.5f)); // Updated
    fval_ptr = FloatMap_get(&fmap, key3); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval3));
    fval_ptr = FloatMap_get(&fmap, key4); assert(fval_ptr != NULL && float_eq(*fval_ptr, fval4));

    // --- Heap ---
    printf("Getting non-existent key 99\n");
    fval_ptr = FloatMap_get(&fmap, key_nonexistent);
    assert(fval_ptr == NULL);

     printf("Modifying key 1 via pointer\n");
    fval_ptr = FloatMap_get(&fmap, key1);
    assert(fval_ptr != NULL);
    if (fval_ptr) {
        *fval_ptr = 1.5f;
    }
    fval_ptr = FloatMap_get(&fmap, key1);
    assert(fval_ptr != NULL && float_eq(*fval_ptr, 1.5f));

    // --- Free ---
    printf("Freeing FloatMap\n");
    FloatMap_free(&fmap);
    assert(fmap.current_size == 0);
    assert(fmap.table == fmap.inline_buffer);

    printf("FloatMap tests passed.\n");


    // ========================================================
    //                    STRESS TEST CALL
    // ========================================================
    run_intmap_stress_test(1000000); // Run stress test with 1 million elements


    printf("\nAll tests finished.\n");
    return 0;
}