#include <stdio.h>

int main(int argc, char *argv[]) {
    // Default file name
    const char* filename = "test_input.txt";
    
    // Check if custom filename was provided
    if (argc > 1) {
        filename = argv[1];
    }
    
    printf("Creating 10-byte test file: %s\n", filename);
    
    // Open file for writing
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open file for writing.\n");
        return 1;
    }
    
    // Write ten 'A' characters to the file
    char bytes[10] = {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'};
    size_t written = fwrite(bytes, 1, 10, file);
    
    if (written != 10) {
        printf("Error: Failed to write to file.\n");
        fclose(file);
        return 1;
    }
    
    printf("File successfully created: 10 bytes written with value 'A' (ASCII %d).\n", bytes[0]);
    
    // Cleanup
    fclose(file);
    
    return 0;
}