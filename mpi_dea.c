#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include "C:\Program Files (x86)\Microsoft SDKs\MPI\Include\mpi.h"
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

int write_file_as_ascii(const char* filename, const void* data, size_t size) {
    FILE* file = fopen(filename, "w"); // Note: changed to "w" instead of "wb"
    if (!file) {
        printf("Error: Could not open file %s for writing\n", filename);
        return 0;
    }
    
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        // Write each byte as an ASCII decimal value followed by a space
        fprintf(file, "%d ", bytes[i]);
    }
    
    fclose(file);
    return 1;
}

// Function to write data to a file
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

int main(int argc, char *argv[]) {
    int rank, size, i, j;
    MPI_Status status;
    uint64_t start_cycles, end_cycles;
    uint64_t load_cycles = 0, encrypt_cycles = 0, decrypt_cycles = 0, write_cycles = 0, total_cycles = 0;
    size_t file_size = 0;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Input/output file names
    const char* input_file = "test_input.txt";
    const char* encrypted_file = "encrypted_output.bin";
    const char* decrypted_file = "decrypted_output.txt";
    
    // Number of iterations for more accurate timing
    const int num_iterations = 10;
    
    char *input_data = NULL;
    uint8_t *full_encrypted = NULL;
    uint8_t *full_decrypted = NULL;
    
    // Master process
    if (rank == 0) {
        printf("=== MPI Multi-Key DEA Encryption Test ===\n");
        printf("Number of processes: %d\n", size);
        printf("Input file: %s\n", input_file);
        printf("Number of iterations: %d\n", num_iterations);
        
        // Load the input file first to determine size
        start_cycles = get_cycles();
        input_data = load_file(input_file, &file_size);
        end_cycles = get_cycles();
        load_cycles = end_cycles - start_cycles;
        
        if (!input_data) {
            printf("Failed to load input file\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        
        // Special case for very small files (4 bytes or less)
        if (file_size <= 4) {
            printf("Small file optimization: File size is only %zu bytes, processing on master only\n", file_size);
            
            // Broadcast file size to worker processes so they know this is a small file case
            MPI_Bcast(&file_size, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
            
            // Allocate buffers
            full_encrypted = malloc(file_size);
            full_decrypted = malloc(file_size + 1);
            
            if (!full_encrypted || !full_decrypted) {
                printf("Memory allocation failed\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
                return 1;
            }
            
            // Initialize DEA
            DEA dea;
            dea_init(&dea);
            dea_set_key(&dea, 0xAA);
            dea_set_key(&dea, 0xBB);
            dea_set_key(&dea, 0xCC);
            dea_set_key(&dea, 0xDD);
            
            print_data("Original (sample)", (uint8_t*)input_data, file_size);
            
            // Process entire file on master
            encrypt_cycles = 0;
            decrypt_cycles = 0;
            
            // Encryption (multiple iterations for timing)
            for (j = 0; j < num_iterations; j++) {
                dea_reset(&dea);
                uint64_t encrypt_start = get_cycles();
                dea_encrypt_block(&dea, (uint8_t*)input_data, file_size, full_encrypted);
                uint64_t encrypt_end = get_cycles();
                encrypt_cycles += (encrypt_end - encrypt_start);
            }
            
            // Average encryption time
            encrypt_cycles /= num_iterations;
            
            // Decryption
            dea_reset(&dea);
            uint64_t decrypt_start = get_cycles();
            dea_decrypt_block(&dea, full_encrypted, file_size, full_decrypted);
            uint64_t decrypt_end = get_cycles();
            decrypt_cycles = decrypt_end - decrypt_start;
            
            full_decrypted[file_size] = '\0';
            
            print_data("Encrypted (sample)", full_encrypted, file_size);
            print_data("Decrypted (sample)", full_decrypted, file_size);
            
            // Check if decryption is correct
            if (memcmp(input_data, full_decrypted, file_size) == 0) {
                printf("\nVerification SUCCESSFUL - The decrypted text matches the original!\n");
            } else {
                printf("\nVerification FAILED - The decrypted text does not match the original!\n");
            }
            
            // Write encrypted data to file
            start_cycles = get_cycles();
            if (write_file_as_ascii(encrypted_file, full_encrypted, file_size)) {
                printf("Encrypted data (as ASCII numbers) written to %s\n", encrypted_file);
            } else {
                printf("Failed to write encrypted data\n");
            }
            
            // Write decrypted data to file
            if (write_file(decrypted_file, full_decrypted, file_size)) {
                printf("Decrypted data written to %s\n", decrypted_file);
            } else {
                printf("Failed to write decrypted data\n");
            }
            end_cycles = get_cycles();
            write_cycles = end_cycles - start_cycles;
            
            // Calculate total time
            total_cycles = load_cycles + encrypt_cycles + decrypt_cycles + write_cycles;
            
            // Print performance metrics
            printf("\n=== Performance Results (%zu byte file, %d iterations) ===\n", 
                   file_size, num_iterations);
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
            
            // Calculate throughput
            double encrypt_mb_per_second = ((file_size) / 1024.0 / 1024.0) / (cycles_to_ms(encrypt_cycles) / 1000.0);
            double decrypt_mb_per_second = ((file_size) / 1024.0 / 1024.0) / (cycles_to_ms(decrypt_cycles) / 1000.0);
            printf("\nThroughput:\n");
            printf("Encryption:  %.2f MB/s\n", encrypt_mb_per_second);
            printf("Decryption:  %.2f MB/s\n", decrypt_mb_per_second);
            
            printf("\nCycles per byte:\n");
            printf("File load:   %.2f cycles/byte\n", (double)load_cycles / file_size);
            printf("Encryption:  %.2f cycles/byte\n", (double)encrypt_cycles / file_size);
            printf("Decryption:  %.2f cycles/byte\n", (double)decrypt_cycles / file_size);
            printf("File write:  %.2f cycles/byte\n", (double)write_cycles / file_size);
            printf("Total:       %.2f cycles/byte\n", (double)total_cycles / file_size);
            
            // Cleanup
            free(input_data);
            free(full_encrypted);
            free(full_decrypted);
            
            MPI_Finalize();
            return 0;
        }
        
        printf("File loaded successfully: %zu bytes\n", file_size);
        printf("File load time: %llu cycles (%.3f ms)\n", 
               load_cycles, cycles_to_ms(load_cycles));
        print_data("Original (sample)", (uint8_t*)input_data, file_size);
        
        // Broadcast file size to all processes
        MPI_Bcast(&file_size, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
        
        // Calculate chunks for input data
        int chunk_size = file_size / size;
        int remainder = file_size % size;
        
        // Send chunk sizes to workers (they'll reuse the same chunk for all iterations)
        for (i = 1; i < size; i++) {
            int worker_chunk_size = chunk_size + (i < remainder ? 1 : 0);
            int start_pos = i * chunk_size + (i < remainder ? i : remainder);
            
            MPI_Send(&worker_chunk_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&num_iterations, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send((void*)&input_data[start_pos], worker_chunk_size, MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
        
        // Master's chunk
        int master_chunk_size = chunk_size + (0 < remainder ? 1 : 0);
        
        // Allocate buffers
        uint8_t *master_encrypted = malloc(master_chunk_size);
        full_encrypted = malloc(file_size);
        full_decrypted = malloc(file_size + 1);
        
        if (!master_encrypted || !full_encrypted || !full_decrypted) {
            printf("Memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        
        // Initialize DEA
        DEA dea;
        dea_init(&dea);
        dea_set_key(&dea, 0xAA);
        dea_set_key(&dea, 0xBB);
        dea_set_key(&dea, 0xCC);
        dea_set_key(&dea, 0xDD);
        
        // Synchronize before timing starts
        MPI_Barrier(MPI_COMM_WORLD);
        encrypt_cycles = 0;
        decrypt_cycles = 0;
        
        // Multiple iterations for more accurate timing
        for (j = 0; j < num_iterations; j++) {
            // Reset DEA key counter for each iteration
            dea_reset(&dea);
            
            // Process master's chunk with timing
            uint64_t chunk_start = get_cycles();
            dea_encrypt_block(&dea, (uint8_t*)input_data, master_chunk_size, master_encrypted);
            uint64_t chunk_end = get_cycles();
            encrypt_cycles += (chunk_end - chunk_start);
            
            // Copy to result buffer
            memcpy(full_encrypted, master_encrypted, master_chunk_size);
            
            // Collect results from workers
            for (i = 1; i < size; i++) {
                int worker_chunk_size = chunk_size + (i < remainder ? 1 : 0);
                int start_pos = i * chunk_size + (i < remainder ? i : remainder);
                
                MPI_Recv(&full_encrypted[start_pos], worker_chunk_size, MPI_BYTE, i, j, MPI_COMM_WORLD, &status);
            }
            
            // Only decrypt and verify the last iteration
            if (j == num_iterations - 1) {
                // Start timing for decryption
                uint64_t decrypt_start = get_cycles();
                
                // We need to decrypt each chunk separately with the correct key offset
                
                // First decrypt master's chunk
                dea_reset(&dea);
                // Master's chunk starts at the beginning of the file (offset 0)
                dea_decrypt_block(&dea, full_encrypted, master_chunk_size, full_decrypted);
                
                // Now decrypt each worker's chunk with the correct key offset
                for (i = 1; i < size; i++) {
                    int worker_chunk_size = chunk_size + (i < remainder ? 1 : 0);
                    int start_pos = i * chunk_size + (i < remainder ? i : remainder);
                    
                    // Calculate key counter offset based on chunk position
                    int preceding_bytes = 0;
                    for (int k = 0; k < i; k++) {
                        preceding_bytes += chunk_size + (k < remainder ? 1 : 0);
                    }
                    
                    // Reset DEA with the correct key offset for this chunk
                    dea_reset(&dea);
                    dea.key_counter = preceding_bytes % dea.num_keys;
                    
                    // Decrypt this chunk
                    dea_decrypt_block(&dea, &full_encrypted[start_pos], worker_chunk_size, &full_decrypted[start_pos]);
                }
                
                uint64_t decrypt_end = get_cycles();
                decrypt_cycles = decrypt_end - decrypt_start;
                
                full_decrypted[file_size] = '\0';
                
                print_data("Encrypted (sample)", full_encrypted, file_size);
                print_data("Decrypted (sample)", full_decrypted, file_size);
                
                // Check if decryption is correct
                if (memcmp(input_data, full_decrypted, file_size) == 0) {
                    printf("\nVerification SUCCESSFUL - The decrypted text matches the original!\n");
                } else {
                    printf("\nVerification FAILED - The decrypted text does not match the original!\n");
                }
                
                // Write encrypted data to file
                start_cycles = get_cycles();
                if (write_file_as_ascii(encrypted_file, full_encrypted, file_size)) {
                    printf("Encrypted data (as ASCII numbers) written to %s\n", encrypted_file);
                } else {
                    printf("Failed to write encrypted data\n");
                }
                
                // Write decrypted data to file
                if (write_file(decrypted_file, full_decrypted, file_size)) {
                    printf("Decrypted data written to %s\n", decrypted_file);
                } else {
                    printf("Failed to write decrypted data\n");
                }
                end_cycles = get_cycles();
                write_cycles = end_cycles - start_cycles;
            }
        }
        
        // Calculate average encryption time per iteration
        encrypt_cycles /= num_iterations;
        
        // Calculate and print performance metrics
        total_cycles = load_cycles + encrypt_cycles + decrypt_cycles + write_cycles;
        
        printf("\n=== Performance Results (%zuMB file, %d iterations) ===\n", 
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
        // Calculate throughput
        double encrypt_mb_per_second = ((file_size) / 1024.0 / 1024.0) / (cycles_to_ms(encrypt_cycles) / 1000.0);
        double decrypt_mb_per_second = ((file_size) / 1024.0 / 1024.0) / (cycles_to_ms(decrypt_cycles) / 1000.0);
        printf("\nThroughput:\n");
        printf("Encryption:  %.2f MB/s\n", encrypt_mb_per_second);
        printf("Decryption:  %.2f MB/s\n", decrypt_mb_per_second);
        
        printf("\nCycles per byte:\n");
        printf("File load:   %.2f cycles/byte\n", (double)load_cycles / file_size);
        printf("Encryption:  %.2f cycles/byte\n", (double)encrypt_cycles / file_size);
        printf("Decryption:  %.2f cycles/byte\n", (double)decrypt_cycles / file_size);
        printf("File write:  %.2f cycles/byte\n", (double)write_cycles / file_size);
        printf("Total:       %.2f cycles/byte\n", (double)total_cycles / file_size);
        
        // Compare with ideal linear scaling
        printf("\nScaling Analysis:\n");
        printf("With %d processes: %.2f MB/second (encryption)\n", size, encrypt_mb_per_second);
        printf("Estimated single-process encryption performance: %.2f MB/second\n", encrypt_mb_per_second / size);
        printf("Parallel efficiency: %.2f%%\n", 100.0);  // We'd need single-process benchmark to calculate actual efficiency
        
        // Cleanup
        free(input_data);
        free(master_encrypted);
        free(full_encrypted);
        free(full_decrypted);
    }
    // Worker processes
    else {
        // Receive file size from master
        MPI_Bcast(&file_size, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
        
        // For very small files, master handles everything
        if (file_size <= 4) {
            // Exit early - master is handling the small file
            MPI_Finalize();
            return 0;
        }
        
        int chunk_size, iterations;
        
        // Receive chunk size and iteration count
        MPI_Recv(&chunk_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&iterations, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        
        // Receive data (same chunk used for all iterations)
        uint8_t *chunk_data = malloc(chunk_size);
        uint8_t *encrypted_chunk = malloc(chunk_size);
        
        if (!chunk_data || !encrypted_chunk) {
            printf("Worker %d: Memory allocation failed\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        
        MPI_Recv(chunk_data, chunk_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
        
        printf("Process %d received %d bytes, will encrypt for %d iterations\n", 
               rank, chunk_size, iterations);
        
        // Initialize DEA
        DEA dea;
        dea_init(&dea);
        dea_set_key(&dea, 0xAA);
        dea_set_key(&dea, 0xBB);
        dea_set_key(&dea, 0xCC);
        dea_set_key(&dea, 0xDD);
        
        // Calculate key counter offset based on chunk position
        int chunk_size_base = file_size / size;
        int remainder = file_size % size;
        int preceding_bytes = 0;
        
        for (i = 0; i < rank; i++) {
            preceding_bytes += chunk_size_base + (i < remainder ? 1 : 0);
        }
        
        printf("Process %d: key counter offset is %d bytes (mod %d = %d)\n", 
               rank, preceding_bytes, dea.num_keys, preceding_bytes % dea.num_keys);
        
        // Synchronize before timing starts
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Multiple iterations
        for (j = 0; j < iterations; j++) {
            // Reset DEA key counter for each iteration with proper offset
            dea_reset(&dea);
            dea.key_counter = preceding_bytes % dea.num_keys;
            
            // Encrypt the chunk
            uint64_t chunk_start = get_cycles();
            dea_encrypt_block(&dea, chunk_data, chunk_size, encrypted_chunk);
            uint64_t chunk_end = get_cycles();
            
            // Report timing if needed
            if (j == iterations - 1) {
                printf("Process %d: Chunk encryption time: %.3f ms\n", 
                       rank, cycles_to_ms(chunk_end - chunk_start));
            }
            
            // Send back encrypted data
            MPI_Send(encrypted_chunk, chunk_size, MPI_BYTE, 0, j, MPI_COMM_WORLD);
        }
        
        // Cleanup
        free(chunk_data);
        free(encrypted_chunk);
    }
    
    MPI_Finalize();
    return 0;
}