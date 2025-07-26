#include <stdio.h>

int main(int argc, char *argv[]) {
    // Default file name
    const char* filename = "test_input.txt";
    
    // Check if custom filename was provided
    if (argc > 1) {
        filename = argv[1];
    }
    
    printf("Creating 100-byte test file: %s\n", filename);
    
    // Open file for writing
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open file for writing.\n");
        return 1;
    }
    
    // Create a 100-byte array filled with 'A' characters
    char bytes[100];
    for (int i = 0; i < 100; i++) {
        bytes[i] = 'A';
    }
    
    // Write 100 'A' characters to the file
    size_t written = fwrite(bytes, 1, 100, file);
    
    if (written != 100) {
        printf("Error: Failed to write to file. Only wrote %zu bytes.\n", written);
        fclose(file);
        return 1;
    }
    
    printf("File successfully created: 100 bytes written with value 'A' (ASCII %d).\n", bytes[0]);
    
    // Cleanup
    fclose(file);
    
    return 0;
}