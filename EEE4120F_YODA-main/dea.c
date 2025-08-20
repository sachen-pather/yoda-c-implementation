#include "dea.h"
#include <string.h>

// Initialize the DEA
void dea_init(DEA *dea) {
    memset(dea->keys, 0, sizeof(dea->keys));
    dea->key_counter = 0;
    dea->num_keys = 0;
    dea->dout = 0;
    dea->initialized = 1;
}

// Reset the DEA state
void dea_reset(DEA *dea) {
    dea->key_counter = 0;
    dea->dout = 0;
    // Note: we don't reset the keys or num_keys on reset
    // This matches the Verilog behavior where reset only resets the counter
    if (!dea->initialized) {
        dea_init(dea);
    }
}

// Set encryption key (load key registers)
void dea_set_key(DEA *dea, uint8_t key) {
    if (!dea->initialized) {
        dea_init(dea);
    }
    
    // Store key in the next available position
    if (dea->num_keys < 4) {
        dea->keys[dea->num_keys] = key;
        dea->num_keys++;
    } else {
        // If we already have 4 keys, start overwriting from the beginning
        dea->keys[0] = key;
        dea->num_keys = 1;
    }
}

// Encrypt a single byte
uint8_t dea_encrypt_byte(DEA *dea, uint8_t data_in) {
    if (!dea->initialized) {
        dea_init(dea);
    }
    
    // Make sure we have at least one key
    if (dea->num_keys == 0) {
        return data_in; // No encryption if no keys are set
    }
    
    // Get the current active key
    uint8_t active_key = dea->keys[dea->key_counter];
    
    // XOR encryption
    dea->dout = data_in ^ active_key;
    
    // Advance to the next key
    dea->key_counter = (dea->key_counter + 1) % dea->num_keys;
    
    return dea->dout;
}

// Encrypt a block of data
void dea_encrypt_block(DEA *dea, uint8_t *data, size_t length, uint8_t *output) {
    for (size_t i = 0; i < length; i++) {
        output[i] = dea_encrypt_byte(dea, data[i]);
    }
}

// Decrypt function - for XOR encryption, we need to reset the key counter
// and then perform the same operation as encryption
void dea_decrypt_block(DEA *dea, uint8_t *data, size_t length, uint8_t *output) {
    // DO NOT reset the key counter - it should be set correctly before calling this function
    // In MPI, different processes encrypt with different key counter offsets
    
    // For XOR encryption, encryption and decryption are the same operation
    // Just apply the encryption algorithm with the current key counter
    dea_encrypt_block(dea, data, length, output);
}