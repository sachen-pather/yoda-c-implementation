#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
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

// Function to get current CPU cycles
uint64_t get_cycles() {
    return __rdtsc();
}

// Function to convert cycles to milliseconds (adjust CPU_FREQ_GHZ for your CPU)
double cycles_to_ms(uint64_t cycles) {
    const double CPU_FREQ_GHZ = 3.0; // Replace with your CPU frequency in GHz
    return (double)cycles / (CPU_FREQ_GHZ * 1.0e9) * 1000.0;
}

// Function to load a file into memory
char* load_file(const char* filename, size_t* file_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate memory
    char* buffer = (char*)malloc(*file_size + 1);
    if (!buffer) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    
    // Read file into buffer
    size_t bytes_read = fread(buffer, 1, *file_size, file);
    if (bytes_read != *file_size) {
        printf("Warning: Read %zu bytes, expected %zu bytes\n", bytes_read, *file_size);
        *file_size = bytes_read;
    }
    buffer[*file_size] = '\0';
    
    fclose(file);
    return buffer;
}

// Function to write binary data to a file
int write_file(const char* filename, const void* data, size_t size) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open file %s for writing\n", filename);
        return 0;
    }
    
    size_t bytes_written = fwrite(data, 1, size, file);
    fclose(file);
    
    if (bytes_written != size) {
        printf("Warning: Wrote %zu bytes, expected %zu bytes\n", bytes_written, size);
        return 0;
    }
    
    return 1;
}

// Function to write data as ASCII decimal values to a file
int write_file_as_ascii(const char* filename, const void* data, size_t size) {
    FILE* file = fopen(filename, "w"); // Text mode, not binary
    if (!file) {
        printf("Error: Could not open file %s for writing\n", filename);
        return 0;
    }
    
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        if (fprintf(file, "%d ", bytes[i]) < 0) {
            printf("Error writing to file at position %zu\n", i);
            fclose(file);
            return 0;
        }
    }
    
    fclose(file);
    return 1;
}

int main() {
    DEA dea;
    dea_init(&dea);
    
    printf("=== Serial Multi-Key DEA Encryption Test ===\n\n");
    
    // Input/output file names
    const char* input_file = "test_input.txt";
    const char* encrypted_file = "serial_encrypted_output.bin";
    const char* decrypted_file = "serial_decrypted_output.txt";
    
    // Number of iterations for more accurate timing of encryption/decryption
    const int num_iterations = 10;
    
    // Variables for timing
    uint64_t load_cycles, encrypt_cycles, decrypt_cycles, write_cycles, total_cycles;
    
    printf("Input file: %s\n", input_file);
    printf("Number of iterations for encryption: %d\n", num_iterations);
    
    // Set up 4 different keys (same as your MPI implementation)
    printf("Setting up 4 encryption keys...\n");
    dea_reset(&dea);
    dea_set_key(&dea, 0xAA);
    dea_set_key(&dea, 0xBB);
    dea_set_key(&dea, 0xCC);
    dea_set_key(&dea, 0xDD);
    
    // Load the input file with timing
    printf("Loading input file...\n");
    size_t file_size = 0;
    uint64_t start_cycles = get_cycles();
    uint8_t *input_data = (uint8_t*)load_file(input_file, &file_size);
    uint64_t end_cycles = get_cycles();
    load_cycles = end_cycles - start_cycles;
    
    if (!input_data) {
        printf("Failed to load input file\n");
        return 1;
    }
    
    printf("File loaded successfully: %zu bytes\n", file_size);
    printf("File load time: %llu cycles (%.3f ms)\n", 
           load_cycles, cycles_to_ms(load_cycles));
    
    print_data("Original (sample)", input_data, file_size);
    
    // Allocate memory for encrypted and decrypted data
    uint8_t *encrypted = malloc(file_size);
    uint8_t *decrypted = malloc(file_size + 1);  // +1 for null terminator
    
    if (!encrypted || !decrypted) {
        printf("Memory allocation failed\n");
        free(input_data);
        return 1;
    }
    
    // Run a small encryption to warm up the cache
    dea_reset(&dea);
    dea_encrypt_block(&dea, input_data, 1024 < file_size ? 1024 : file_size, encrypted);
    
    // Start the encryption benchmark
    printf("\nStarting encryption benchmark (%zu bytes × %d iterations)...\n", 
           file_size, num_iterations);
    
    encrypt_cycles = 0;
    
    // Multiple iterations for more accurate timing
    for (int j = 0; j < num_iterations; j++) {
        dea_reset(&dea);
        start_cycles = get_cycles();
        dea_encrypt_block(&dea, input_data, file_size, encrypted);
        end_cycles = get_cycles();
        encrypt_cycles += (end_cycles - start_cycles);
    }
    
    // Calculate average encryption time
    encrypt_cycles /= num_iterations;
    
    // Show a sample of the encrypted data
    print_data("Encrypted (sample)", encrypted, file_size);
    
    // Verify with decryption
    printf("\nPerforming decryption...\n");
    dea_reset(&dea);
    
    start_cycles = get_cycles();
    dea_decrypt_block(&dea, encrypted, file_size, decrypted);
    end_cycles = get_cycles();
    decrypt_cycles = end_cycles - start_cycles;
    
    decrypted[file_size] = '\0';
    
    print_data("Decrypted (sample)", decrypted, file_size);
    
    // Verify correctness
    if (memcmp(input_data, decrypted, file_size) == 0) {
        printf("\nVerification SUCCESSFUL - The decrypted text matches the original!\n");
    } else {
        printf("\nVerification FAILED - The decrypted text does not match the original!\n");
    }
    
    // Write encrypted and decrypted data to files
    printf("\nWriting output files...\n");
    start_cycles = get_cycles();
    int write_success = 1;
    
    // Write encrypted data as ASCII decimal values
    if (write_file_as_ascii(encrypted_file, encrypted, file_size)) {
        printf("Encrypted data (as ASCII decimal values) written to %s\n", encrypted_file);
    } else {
        printf("Failed to write encrypted data\n");
        write_success = 0;
    }
    
    // Write decrypted data as normal text
    if (write_file(decrypted_file, decrypted, file_size)) {
        printf("Decrypted data written to %s\n", decrypted_file);
    } else {
        printf("Failed to write decrypted data\n");
        write_success = 0;
    }
    
    end_cycles = get_cycles();
    write_cycles = end_cycles - start_cycles;
    
    // Calculate total time
    total_cycles = load_cycles + encrypt_cycles + decrypt_cycles + write_cycles;
    
    // Print performance metrics
    printf("\n=== Performance Results (%zu.serial_dea.exeB file, %d iterations) ===\n", 
           (file_size / (1024 * 1024)) + ((file_size % (1024 * 1024)) ? 1 : 0), 
           num_iterations);
    printf("File load:     %llu cycles (%.3f ms) (%.3f%% of total)\n", 
           load_cycles, 
           cycles_to_ms(load_cycles), 
           (double)load_cycles / total_cycles * 100.0);
    printf("Encryption:    %llu cycles (%.3f ms) (%.3f%% of total)\n", 
           encrypt_cycles, 
           cycles_to_ms(encrypt_cycles), 
           (double)encrypt_cycles / total_cycles * 100.0);
    printf("Decryption:    %llu cycles (%.3f ms) (%.3f%% of total)\n", 
           decrypt_cycles, 
           cycles_to_ms(decrypt_cycles), 
           (double)decrypt_cycles / total_cycles * 100.0);
    printf("File write:    %llu cycles (%.3f ms) (%.3f%% of total)\n", 
           write_cycles, 
           cycles_to_ms(write_cycles), 
           (double)write_cycles / total_cycles * 100.0);
    printf("Total:         %llu cycles (%.3f ms)\n", 
           total_cycles,
           cycles_to_ms(total_cycles));
    
    printf("\nCycles per byte:\n");
    printf("File load:   %.2f cycles/byte\n", (double)load_cycles / file_size);
    printf("Encryption:  %.2f cycles/byte\n", (double)encrypt_cycles / file_size);
    printf("Decryption:  %.2f cycles/byte\n", (double)decrypt_cycles / file_size);
    printf("File write:  %.2f cycles/byte\n", (double)write_cycles / file_size);
    printf("Total:       %.2f cycles/byte\n", (double)total_cycles / file_size);

    // Cleanup
    free(input_data);
    free(encrypted);
    free(decrypted);
    
    return 0;
}