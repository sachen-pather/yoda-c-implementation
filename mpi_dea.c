#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

int main(int argc, char *argv[]) {
    int rank, size, i, j;
    MPI_Status status;
    clock_t start_time, end_time;
    double process_time, total_time;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Test data size
    const size_t test_size = 10 * 1024 * 1024; // 10MB
    // Number of iterations for more accurate timing
    const int num_iterations = 10;
    
    char *large_message = NULL;
    uint8_t *full_encrypted = NULL;
    uint8_t *full_decrypted = NULL;
    
    // Master process
    if (rank == 0) {
        printf("=== MPI Multi-Key DEA Encryption Test ===\n");
        printf("Number of processes: %d\n", size);
        printf("Test size: %zu bytes\n", test_size);
        printf("Number of iterations: %d\n", num_iterations);
        
        // Create the large test message
        large_message = (char*)malloc(test_size + 1);
        if (!large_message) {
            printf("Memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        
        // Fill with repeating pattern
        for (size_t i = 0; i < test_size; i++) {
            large_message[i] = 'A' + (i % 26);
        }
        large_message[test_size] = '\0';
        
        print_data("Original (sample)", (uint8_t*)large_message, test_size);
        
        // Calculate chunks for test message
        int chunk_size = test_size / size;
        int remainder = test_size % size;
        
        // Send chunk sizes to workers (they'll reuse the same chunk for all iterations)
        for (i = 1; i < size; i++) {
            int worker_chunk_size = chunk_size + (i < remainder ? 1 : 0);
            int start_pos = i * chunk_size + (i < remainder ? i : remainder);
            
            MPI_Send(&worker_chunk_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&num_iterations, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send((void*)&large_message[start_pos], worker_chunk_size, MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
        
        // Master's chunk
        int master_chunk_size = chunk_size + (0 < remainder ? 1 : 0);
        
        // Allocate buffers
        uint8_t *master_encrypted = malloc(master_chunk_size);
        full_encrypted = malloc(test_size);
        full_decrypted = malloc(test_size + 1);
        
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
        start_time = clock();
        
        // Multiple iterations for more accurate timing
        for (j = 0; j < num_iterations; j++) {
            // Reset DEA for each iteration
            dea_reset(&dea);
            
            // Process master's chunk
            dea_encrypt_block(&dea, (uint8_t*)large_message, master_chunk_size, master_encrypted);
            
            // Copy to result buffer
            memcpy(full_encrypted, master_encrypted, master_chunk_size);
            
            // Collect results from workers
            for (i = 1; i < size; i++) {
                int worker_chunk_size = chunk_size + (i < remainder ? 1 : 0);
                int start_pos = i * chunk_size + (i < remainder ? i : remainder);
                
                MPI_Recv(&full_encrypted[start_pos], worker_chunk_size, MPI_BYTE, i, j, MPI_COMM_WORLD, &status);
            }
            
            // Only verify the last iteration
            if (j == num_iterations - 1) {
                // Decrypt for verification
                dea_reset(&dea);
                dea_decrypt_block(&dea, full_encrypted, test_size, full_decrypted);
                full_decrypted[test_size] = '\0';
                
                print_data("Encrypted (sample)", full_encrypted, test_size);
                print_data("Decrypted (sample)", full_decrypted, test_size);
                
                // Check if decryption is correct
                if (memcmp(large_message, full_decrypted, test_size) == 0) {
                    printf("\nVerification SUCCESSFUL - The decrypted text matches the original!\n");
                } else {
                    printf("\nVerification FAILED - The decrypted text does not match the original!\n");
                }
            }
        }
        
        // Get end time
        end_time = clock();
        total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
        
        // Calculate and print performance metrics
        printf("\n=== Performance Results (%dMB Test, %d iterations) ===\n", 
               (int)(test_size / (1024 * 1024)), num_iterations);
        printf("Total execution time: %.3f ms\n", total_time);
        printf("Average time per iteration: %.3f ms\n", total_time / num_iterations);
        printf("Total data processed: %zu bytes\n", test_size * num_iterations);
        
        double mb_per_second = ((test_size * num_iterations) / 1024.0 / 1024.0) / (total_time / 1000.0);
        printf("Throughput: %.2f MB/second\n", mb_per_second);
        
        // Compare with ideal linear scaling
        printf("\nScaling Analysis:\n");
        printf("With %d processes: %.2f MB/second\n", size, mb_per_second);
        printf("Estimated single-process performance: %.2f MB/second\n", mb_per_second / size);
        printf("Parallel efficiency: %.2f%%\n", (mb_per_second / size * 100.0) / mb_per_second);
        
        // Cleanup
        free(large_message);
        free(master_encrypted);
        free(full_encrypted);
        free(full_decrypted);
    }
    // Worker processes
    else {
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
        
        // Calculate key counter offset
        int preceding_bytes = 0;
        for (i = 0; i < rank; i++) {
            preceding_bytes += (test_size / size) + (i < (test_size % size) ? 1 : 0);
        }
        dea.key_counter = preceding_bytes % dea.num_keys;
        
        // Synchronize before timing starts
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Multiple iterations
        for (j = 0; j < iterations; j++) {
            // Reset DEA key counter for each iteration
            dea.key_counter = preceding_bytes % dea.num_keys;
            
            // Encrypt the chunk
            dea_encrypt_block(&dea, chunk_data, chunk_size, encrypted_chunk);
            
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