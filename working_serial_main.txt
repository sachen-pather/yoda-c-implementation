#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "dea.h"

// Function to print data as both hex and as a string (truncated for large data)
void print_data(const char* label, uint8_t *data, size_t length) {
    printf("%s (hex): ", label);
    // Only print the first 20 bytes
    size_t display_length = length > 20 ? 20 : length;
    for (size_t i = 0; i < display_length; i++) {
        printf("%02X ", data[i]);
    }
    if (length > 20) printf("... (truncated)");
    printf("\n");
    
    printf("%s (text): \"", label);
    // Only print the first 40 characters
    display_length = length > 40 ? 40 : length;
    for (size_t i = 0; i < display_length; i++) {
        if (data[i] >= 32 && data[i] <= 126) {
            printf("%c", data[i]);
        } else {
            printf(".");
        }
    }
    if (length > 40) printf("...\"");
    else printf("\"");
    printf("\n");
}

int main() {
    DEA dea;
    dea_init(&dea);
    
    printf("=== Multi-Key DEA Encryption Performance Test ===\n\n");
    
    // Test parameters (same as the other implementation)
    const size_t test_size = 10 * 1024 * 1024; // 10MB
    const int num_iterations = 10;
    
    printf("Test size: %zu bytes\n", test_size);
    printf("Number of iterations: %d\n", num_iterations);
    
    // Set up 4 different keys
    printf("Setting up 4 encryption keys...\n");
    dea_reset(&dea);
    dea_set_key(&dea, 0xAA);
    dea_set_key(&dea, 0xBB);
    dea_set_key(&dea, 0xCC);
    dea_set_key(&dea, 0xDD);
    
    // Create test data
    uint8_t *large_data = malloc(test_size);
    uint8_t *encrypted = malloc(test_size);
    uint8_t *decrypted = malloc(test_size + 1);  // +1 for null terminator
    
    if (!large_data || !encrypted || !decrypted) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Fill with repeating pattern (same as the other implementation)
    for (size_t i = 0; i < test_size; i++) {
        large_data[i] = 'A' + (i % 26);
    }
    
    print_data("Original (sample)", large_data, test_size);
    
    // Run a small encryption to warm up the cache
    dea_reset(&dea);
    dea_encrypt_block(&dea, large_data, 1024, encrypted);
    
    // Start the benchmark
    printf("\nStarting benchmark (%d MB × %d iterations)...\n", 
           (int)(test_size / (1024 * 1024)), num_iterations);
    
    clock_t start_time = clock();
    
    // Multiple iterations for more accurate timing
    for (int j = 0; j < num_iterations; j++) {
        dea_reset(&dea);
        dea_encrypt_block(&dea, large_data, test_size, encrypted);
    }
    
    clock_t end_time = clock();
    double total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    
    // Show a sample of the encrypted data
    print_data("Encrypted (sample)", encrypted, test_size);
    
    // Verify with decryption
    dea_reset(&dea);
    
    clock_t decrypt_start = clock();
    dea_decrypt_block(&dea, encrypted, test_size, decrypted);
    clock_t decrypt_end = clock();
    double decrypt_time = ((double)(decrypt_end - decrypt_start)) / CLOCKS_PER_SEC * 1000.0;
    
    decrypted[test_size] = '\0';
    
    print_data("Decrypted (sample)", decrypted, test_size);
    
    // Verify correctness
    if (memcmp(large_data, decrypted, test_size) == 0) {
        printf("\nVerification SUCCESSFUL - The decrypted text matches the original!\n");
    } else {
        printf("\nVerification FAILED - The decrypted text does not match the original!\n");
    }
    
    // Print performance metrics
    printf("\n=== Performance Results (%d MB Test, %d iterations) ===\n", 
           (int)(test_size / (1024 * 1024)), num_iterations);
    printf("Total execution time: %.3f ms\n", total_time);
    printf("Average time per iteration: %.3f ms\n", total_time / num_iterations);
    printf("Total data processed: %zu bytes\n", test_size * num_iterations);
    
    double mb_per_second = ((test_size * num_iterations) / 1024.0 / 1024.0) / (total_time / 1000.0);
    printf("Throughput: %.2f MB/second\n", mb_per_second);
    
    // Additional information about decryption performance
    printf("\nDecryption performance:\n");
    printf("Single decryption time: %.3f ms\n", decrypt_time);
    printf("Decryption throughput: %.2f MB/second\n", 
           (test_size / 1024.0 / 1024.0) / (decrypt_time / 1000.0));
    
    // Run original small test to demonstrate key cycling
    printf("\n=== Key Cycling Demonstration ===\n");
    dea_reset(&dea);
    
    uint8_t test_data[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t result;
    
    for (int i = 0; i < 5; i++) {
        result = dea_encrypt_byte(&dea, test_data[i]);
        int key_idx = i % 4;
        uint8_t key_used = (key_idx == 0) ? 0xAA : 
                          (key_idx == 1) ? 0xBB : 
                          (key_idx == 2) ? 0xCC : 0xDD;
                          
        printf("Input: 0x%02X, Key: 0x%02X, Output: 0x%02X\n", 
               test_data[i], key_used, result);
    }
    
    // Cleanup
    free(large_data);
    free(encrypted);
    free(decrypted);
    
    return 0;
}