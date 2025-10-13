#include <stdio.h>
#include <stdbool.h>
#include "N_32.lib.h"  // Include the module under test

// Test function prototypes
bool test_copy();
bool test_bitwise_operations();
bool test_comparisons();
bool test_arithmetic();
bool test_shifts();

// Test array (null-terminated)
typedef bool (*TestFunction)();
typedef struct {
    TestFunction function;
    const char *name;
} TestEntry;

TestEntry test_list[] = {
    { test_copy, "test_copy" },
    { test_bitwise_operations, "test_bitwise_operations" },
    { test_comparisons, "test_comparisons" },
    { test_arithmetic, "test_arithmetic" },
    { test_shifts, "test_shifts" },
    { NULL, NULL }  // Null termination
};

// The test runner
int test_head() {
    int pass_count = 0;
    int fail_count = 0;

    for (TestEntry *entry = test_list; entry->function != NULL; entry++) {
        if (!entry->function()) {
            printf("Failed: %s\n", entry->name);
            fail_count++;
        } else {
            pass_count++;
        }
    }

    printf("Tests passed: %d\n", pass_count);
    printf("Tests failed: %d\n", fail_count);
    return (fail_count == 0) ? 0 : 1;
}

// Main function
int main(int argc, char **argv) {
    return test_head();
}

//------------------------------------------------------------------------------
// Test Implementations
//------------------------------------------------------------------------------

bool test_copy() {
    N_32 a = { .d0 = 42 };
    N_32 b;
    N_32·copy(&b, &a);
    return b.d0 == 42;
}

bool test_bitwise_operations() {
    N_32 a = { .d0 = 0b101010 };
    N_32 b = { .d0 = 0b010101 };
    N_32 result;

    N_32·bit_and(&result, &a, &b);
    if (result.d0 != 0b000000) return false;

    N_32·bit_or(&result, &a, &b);
    if (result.d0 != 0b111111) return false;

    N_32·bit_complement(&result, &a);
    if (result.d0 != ~0b101010) return false;

    return true;
}

bool test_comparisons() {
    N_32 a = { .d0 = 42 };
    N_32 b = { .d0 = 42 };
    N_32 c = { .d0 = 24 };

    if (!N_32·eq(&a, &b)) return false;
    if (N_32·eq(&a, &c)) return false;
    if (!N_32·lt(&c, &a)) return false;
    if (!N_32·gt(&a, &c)) return false;

    return true;
}

bool test_arithmetic() {
    N_32 a = { .d0 = 20 };
    N_32 b = { .d0 = 22 };
    N_32 result;

    if (N_32·add(&result, &a, &b) != N_32·Status·ok) return false;
    if (result.d0 != 42) return false;

    if (N_32·subtract(&result, &b, &a) != N_32·Status·ok) return false;
    if (result.d0 != 2) return false;

    return true;
}

bool test_shifts() {
    N_32 value = { .d0 = 1 };
    N_32 spill;
    
    if (N_32·shift_left(1, &spill, &value, NULL) != N_32·Status·ok) return false;
    if (value.d0 != 2) return false;
    
    if (N_32·shift_right(1, &spill, &value, NULL) != N_32·Status·ok) return false;
    if (value.d0 != 1) return false;

    return true;
}
