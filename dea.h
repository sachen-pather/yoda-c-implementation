#ifndef DEA_H
#define DEA_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t keys[4];          // Storage for 4 8-bit keys
    uint8_t key_counter;      // Counter to cycle through keys (0-3)
    uint8_t num_keys;         // Number of keys stored (0-4)
    uint8_t dout;             // Current output value
    int initialized;          // Flag to track initialization
} DEA;

// Core DEA functions
void dea_init(DEA *dea);
void dea_reset(DEA *dea);
void dea_set_key(DEA *dea, uint8_t key);
uint8_t dea_encrypt_byte(DEA *dea, uint8_t data_in);
void dea_encrypt_block(DEA *dea, uint8_t *data, size_t length, uint8_t *output);
void dea_decrypt_block(DEA *dea, uint8_t *data, size_t length, uint8_t *output);

#endif // DEA_H