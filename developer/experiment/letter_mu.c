#include <stdio.h>

// Define interface and instance types
typedef struct {
    void (*print_value)(int);
} N32·M;  // Interface type

typedef struct {
    int value;
} N32·μ;  // Instance type

// Function implementation for the interface
void print_value_function(int value) {
    printf("Value: %d\n", value);
}

// Default interface instance
const N32·M N32·m = {
    .print_value = print_value_function
};

int main() {
    // Create an instance
    N32·μ instance = { 99 };

    // Call function via interface
    printf("Calling via interface: ");
    N32·m.print_value(instance.value);

    return 0;
}
