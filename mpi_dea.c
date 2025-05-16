#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include "dea.h"

// Function to print data as both hex and as a string
void print_data(const char* label, uint8_t *data, size_t length) {
    printf("%s (hex): ", label);
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    
    printf("%s (text): \"", label);
    for (size_t i = 0; i < length; i++) {
        // Only print printable ASCII characters, use dots for others
        if (data[i] >= 32 && data[i] <= 126) {
            printf("%c", data[i]);
        } else {
            printf(".");
        }
    }
    printf("\"\n");
}

int main(int argc, char *argv[]) {
    int rank, size, i;
    MPI_Status status;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Test message
    const char *message = "mmmmmmmmmmmmmmmmmmmm";
    size_t msg_len = strlen(message);
    
    // Master process
    if (rank == 0) {
        printf("=== MPI Multi-Key DEA Encryption Test ===\n");
        printf("Number of processes: %d\n", size);
        printf("Original message: \"%s\"\n", message);
        print_data("Original", (uint8_t*)message, msg_len);
        
        // Calculate chunks for each process
        int chunk_size = msg_len / size;
        int remainder = msg_len % size;
        
        // Send chunks to workers
        for (i = 1; i < size; i++) {
            int worker_chunk_size = chunk_size + (i <= remainder ? 1 : 0);
            int start_pos = i * chunk_size + (i <= remainder ? i : remainder);
            
            MPI_Send(&worker_chunk_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send((void*)&message[start_pos], worker_chunk_size, MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
        
        // Master's chunk
        int master_chunk_size = chunk_size + (0 < remainder ? 1 : 0);
        
        // Initialize DEA and set keys
        DEA dea;
        dea_init(&dea);
        dea_set_key(&dea, 0xAA);
        dea_set_key(&dea, 0xBB);
        dea_set_key(&dea, 0xCC);
        dea_set_key(&dea, 0xDD);
        
        // Process master's chunk
        uint8_t *master_encrypted = malloc(master_chunk_size);
        dea_encrypt_block(&dea, (uint8_t*)message, master_chunk_size, master_encrypted);
        
        printf("\nProcess %d encrypted %d bytes\n", rank, master_chunk_size);
        
        // Prepare buffer for complete encrypted message
        uint8_t *full_encrypted = malloc(msg_len);
        memcpy(full_encrypted, master_encrypted, master_chunk_size);
        
        // Collect results from workers
        for (i = 1; i < size; i++) {
            int worker_chunk_size = chunk_size + (i <= remainder ? 1 : 0);
            int start_pos = i * chunk_size + (i <= remainder ? i : remainder);
            
            MPI_Recv(&full_encrypted[start_pos], worker_chunk_size, MPI_BYTE, i, 0, MPI_COMM_WORLD, &status);
            printf("Received encrypted chunk from process %d\n", i);
        }
        
        // Print full encrypted result
        printf("\nFull encrypted result:\n");
        print_data("Encrypted", full_encrypted, msg_len);
        
        // Decrypt for verification
        uint8_t *full_decrypted = malloc(msg_len + 1);
        dea_reset(&dea);
        dea_decrypt_block(&dea, full_encrypted, msg_len, full_decrypted);
        full_decrypted[msg_len] = '\0';
        
        printf("\nDecryption verification:\n");
        print_data("Decrypted", full_decrypted, msg_len);
        printf("Decrypted text: \"%s\"\n", full_decrypted);
        
        // Check if decryption is correct
        if (memcmp(message, full_decrypted, msg_len) == 0) {
            printf("\nVerification SUCCESSFUL - The decrypted text matches the original!\n");
        } else {
            printf("\nVerification FAILED - The decrypted text does not match the original!\n");
        }
        
        // Cleanup
        free(master_encrypted);
        free(full_encrypted);
        free(full_decrypted);
    }
    // Worker processes
    else {
        int chunk_size;
        
        // Receive chunk size
        MPI_Recv(&chunk_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        
        // Receive data
        uint8_t *chunk_data = malloc(chunk_size);
        MPI_Recv(chunk_data, chunk_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
        
        printf("Process %d received %d bytes to encrypt\n", rank, chunk_size);
        
        // Initialize DEA and set keys
        DEA dea;
        dea_init(&dea);
        dea_set_key(&dea, 0xAA);
        dea_set_key(&dea, 0xBB);
        dea_set_key(&dea, 0xCC);
        dea_set_key(&dea, 0xDD);
        
        // Calculate key counter offset based on position in message
        int preceding_bytes = 0;
        for (i = 0; i < rank; i++) {
            preceding_bytes += msg_len / size + (i < (msg_len % size) ? 1 : 0);
        }
        
        // Set the key counter to the correct position
        dea.key_counter = preceding_bytes % dea.num_keys;
        
        // Encrypt the chunk
        uint8_t *encrypted_chunk = malloc(chunk_size);
        dea_encrypt_block(&dea, chunk_data, chunk_size, encrypted_chunk);
        
        printf("Process %d encrypted chunk successfully\n", rank);
        
        // Send back to master
        MPI_Send(encrypted_chunk, chunk_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
        
        // Cleanup
        free(chunk_data);
        free(encrypted_chunk);
    }
    
    MPI_Finalize();
    return 0;
}