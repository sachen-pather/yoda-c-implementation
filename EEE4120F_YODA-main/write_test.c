#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    // Default file name and size
    const char* filename = "test_input.txt";
    size_t test_size = 10 * 1024 * 1024; // 10MB
    
    // Check if custom filename was provided
    if (argc > 1) {
        filename = argv[1];
    }
    
    // Check if custom size was provided (in MB)
    if (argc > 2) {
        test_size = atoi(argv[2]) * 1024 * 1024;
    }
    
    printf("Creating test file: %s\n", filename);
    printf("Size: %zu bytes (%.2f MB)\n", test_size, test_size / (1024.0 * 1024.0));
    
    // Open file for writing
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open file for writing.\n");
        return 1;
    }
    
    // Create buffer for more efficient writing
    const size_t buffer_size = 64 * 1024; // 64KB buffer
    char *buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        return 1;
    }
    
    // Fill buffer with pattern (same as in original code)
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = 'A';
    }
    
    // Write buffer to file repeatedly until reaching test_size
    size_t bytes_written = 0;
    while (bytes_written < test_size) {
        size_t remaining = test_size - bytes_written;
        size_t to_write = (remaining < buffer_size) ? remaining : buffer_size;
        
        size_t written = fwrite(buffer, 1, to_write, file);
        if (written != to_write) {
            printf("Error: Failed to write to file.\n");
            free(buffer);
            fclose(file);
            return 1;
        }
        
        bytes_written += written;
        
        // Show progress
        if (bytes_written % (1024 * 1024) == 0) {
            printf("Progress: %.2f MB / %.2f MB\r", 
                  bytes_written / (1024.0 * 1024.0), 
                  test_size / (1024.0 * 1024.0));
            fflush(stdout);
        }
    }
    
    printf("\nFile successfully created: %zu bytes written.\n", bytes_written);
    
    // Cleanup
    free(buffer);
    fclose(file);
    
    return 0;
}