class DEA:
    def __init__(self):
        self.keys = [0] * 4
        self.key_counter = 0
        self.num_keys = 0
        self.dout = 0
        self.initialized = True
    
    def reset(self):
        """Reset the DEA state (key counter only)"""
        self.key_counter = 0
        self.dout = 0
        if not hasattr(self, 'initialized') or not self.initialized:
            self.__init__()
    
    def set_key(self, key):
        """Set encryption key (load key registers)"""
        if not hasattr(self, 'initialized') or not self.initialized:
            self.__init__()
            
        # Store key in the next available position
        if self.num_keys < 4:
            self.keys[self.num_keys] = key
            self.num_keys += 1
        else:
            # If we already have 4 keys, start overwriting from the beginning
            self.keys[0] = key
            self.num_keys = 1
    
    def encrypt_byte(self, data_in):
        """Encrypt a single byte"""
        if not hasattr(self, 'initialized') or not self.initialized:
            self.__init__()
            
        # Make sure we have at least one key
        if self.num_keys == 0:
            return data_in  # No encryption if no keys are set
            
        # Get the current active key
        active_key = self.keys[self.key_counter]
        
        # XOR encryption
        self.dout = data_in ^ active_key
        
        # Advance to the next key
        self.key_counter = (self.key_counter + 1) % self.num_keys
        
        return self.dout
    
    def encrypt_block(self, data, length=None):
        """Encrypt a block of data
        
        Args:
            data: List or bytes object containing the data to encrypt
            length: Optional length to encrypt (defaults to entire data)
            
        Returns:
            Bytearray of encrypted data
        """
        if length is None:
            length = len(data)
        
        output = bytearray(length)
        for i in range(length):
            output[i] = self.encrypt_byte(data[i])
        
        return output
    
    def decrypt_block(self, data, length=None):
        """Decrypt a block of data
        
        For XOR encryption, decryption is the same as encryption
        but we need to reset the key counter first.
        
        Args:
            data: List or bytes object containing the data to decrypt
            length: Optional length to decrypt (defaults to entire data)
            
        Returns:
            Bytearray of decrypted data
        """
        # Save the current key counter
        saved_counter = self.key_counter
        
        # Reset key counter to ensure we start with the same key sequence
        self.key_counter = 0
        
        # For XOR encryption, encryption and decryption are the same operation
        if length is None:
            length = len(data)
        
        output = self.encrypt_block(data, length)
        
        # We typically don't restore the counter as per the C implementation
        # self.key_counter = saved_counter
        
        return output