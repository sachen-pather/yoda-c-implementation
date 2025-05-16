#include <stdio.h>
#include <string.h>
#include "dea.h"

// Utility function to print hex values
void print_hex(uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

// Function to print data as both hex and as a string
void print_data(const char* label, uint8_t *data, size_t length) {
    printf("%s (hex): ", label);
    print_hex(data, length);
    
    printf("%s (text): \"", label);
    for (size_t i = 0; i < length; i++) {
        // Only print printable ASCII characters, use dots for others
        if (data[i] >= 32 && data[i] <= 126) {
            printf("%c", data[i]);
        } else {
            printf(".");  // Use dot for non-printable characters
        }
    }
    printf("\"\n");
}

int main() {
    DEA dea;
    dea_init(&dea);
    
    printf("=== Multi-Key Data Encryption Accelerator Test ===\n\n");
    
    // Set up 4 different keys
    printf("Setting up 4 encryption keys...\n");
    dea_reset(&dea);
    dea_set_key(&dea, 0xAA);
    dea_set_key(&dea, 0xBB);
    dea_set_key(&dea, 0xCC);
    dea_set_key(&dea, 0xDD);
    
    // Data to encrypt
    const char *message = "AAAAAAAAAAAAAAAA!";
    size_t msg_len = strlen(message);
    uint8_t *encrypted = malloc(msg_len);
    uint8_t *decrypted = malloc(msg_len + 1);  // +1 for null terminator
    
    printf("\nOriginal message: \"%s\"\n", message);
    print_data("Original", (uint8_t*)message, msg_len);
    
    // Encrypt
    printf("\nEncrypting with key cycling (0xAA, 0xBB, 0xCC, 0xDD)...\n");
    dea_reset(&dea);
    dea_encrypt_block(&dea, (uint8_t*)message, msg_len, encrypted);
    
    print_data("Encrypted", encrypted, msg_len);
    
    // Decrypt
    printf("\nDecrypting...\n");
    dea_reset(&dea);
    dea_decrypt_block(&dea, encrypted, msg_len, decrypted);
    decrypted[msg_len] = '\0';  // Add null terminator for proper string display
    
    printf("Decrypted: \"%s\"\n", decrypted);
    print_data("Decrypted", decrypted, msg_len);
    
    // Demonstrate individual byte encryption with key cycling
    printf("\n=== Key Cycling Demonstration ===\n");
    dea_reset(&dea);
    
    uint8_t test_data[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t result;
    
    for (int i = 0; i < 5; i++) {
        result = dea_encrypt_byte(&dea, test_data[i]);
        // Calculate which key was used (for display purposes)
        int key_idx = i % 4;
        uint8_t key_used = (key_idx == 0) ? 0xAA : 
                          (key_idx == 1) ? 0xBB : 
                          (key_idx == 2) ? 0xCC : 0xDD;
                          
        printf("Input: 0x%02X, Key: 0x%02X, Output: 0x%02X\n", 
               test_data[i], key_used, result);
    }
    
    // Cleanup
    free(encrypted);
    free(decrypted);
    
    return 0;
}