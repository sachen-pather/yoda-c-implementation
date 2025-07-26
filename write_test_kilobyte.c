#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Default file name
    const char* filename = "test_input.txt";
    
    // Check if custom filename was provided
    if (argc > 1) {
        filename = argv[1];
    }
    
    printf("Creating 1KB (1024-byte) test file: %s\n", filename);
    
    // Open file for writing
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open file for writing.\n");
        return 1;
    }
    
    // Create a 1KB array filled with 'A' characters
    const int size = 1024; // 1KB = 1024 bytes
    char *bytes = (char*)malloc(size);
    
    if (!bytes) {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        return 1;
    }
    
    // Fill the array with 'A' characters
    for (int i = 0; i < size; i++) {
        bytes[i] = 'A';
    }
    
    // Write 1024 'A' characters to the file
    size_t written = fwrite(bytes, 1, size, file);
    
    if (written != size) {
        printf("Error: Failed to write to file. Only wrote %zu bytes of %d.\n", written, size);
        free(bytes);
        fclose(file);
        return 1;
    }
    
    printf("File successfully created: %d bytes written with value 'A' (ASCII %d).\n", size, bytes[0]);
    
    // Cleanup
    free(bytes);
    fclose(file);
    
    return 0;
}