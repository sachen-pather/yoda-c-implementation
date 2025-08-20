#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Default file name
    const char* filename = "test_input.txt";
    
    // Check if custom filename was provided
    if (argc > 1) {
        filename = argv[1];
    }
    
    // 1MB = 1024 * 1024 bytes
    const size_t size = 1024 * 1024; 
    
    printf("Creating 1MB (%zu-byte) test file: %s\n", size, filename);
    
    // Open file for writing
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open file for writing.\n");
        return 1;
    }
    
    // Create a buffer for efficient writing
    // We'll use a smaller buffer and write in chunks to be memory-efficient
    const size_t buffer_size = 64 * 1024; // 64KB buffer
    char *buffer = (char*)malloc(buffer_size);
    
    if (!buffer) {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        return 1;
    }
    
    // Fill the buffer with 'A' characters
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = 'A';
    }
    
    // Write the buffer multiple times until we reach 1MB
    size_t bytes_written = 0;
    while (bytes_written < size) {
        size_t remaining = size - bytes_written;
        size_t chunk_size = (remaining < buffer_size) ? remaining : buffer_size;
        
        size_t written = fwrite(buffer, 1, chunk_size, file);
        if (written != chunk_size) {
            printf("Error: Failed to write to file. Only wrote %zu bytes of %zu.\n", 
                   bytes_written + written, size);
            free(buffer);
            fclose(file);
            return 1;
        }
        
        bytes_written += written;
        
        // Show progress for large files (update every 256KB)
        if (bytes_written % (256 * 1024) == 0 || bytes_written == size) {
            printf("Progress: %.1f%% (%zu KB of %zu KB)\r", 
                  (bytes_written * 100.0) / size, 
                  bytes_written / 1024, 
                  size / 1024);
            fflush(stdout);
        }
    }
    
    printf("\nFile successfully created: %zu bytes (%.2f MB) written with value 'A' (ASCII %d).\n", 
           bytes_written, bytes_written / (1024.0 * 1024.0), 'A');
    
    // Cleanup
    free(buffer);
    fclose(file);
    
    return 0;
}