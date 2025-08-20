import os
import time
import platform
import sys
from dea import DEA

# Function to get CPU cycles
def get_cycles():
    """Get actual CPU cycles using the rdtsc extension"""
    try:
        import rdtsc
        return rdtsc.rdtsc()
    except ImportError:
        print("Warning: rdtsc module not found. Using time-based approximation instead.")
        print("To get actual CPU cycles, build and install the rdtsc extension:")
        print("  python setup.py build")
        print("  python setup.py install")
        # Fallback to timing-based approximation
        return int(time.perf_counter_ns() * 3.0)  # Assuming 3.0 GHz CPU

# Function to print data as both hex and as a string
def print_data(label, data, length=None):
    if length is None:
        length = len(data)
    
    # Hex display
    print(f"{label} (hex): ", end="")
    display_length = min(length, 20)
    for i in range(display_length):
        print(f"{data[i]:02X} ", end="")
    if length > 20:
        print("... (truncated)")
    else:
        print()
    
    # Text display
    print(f"{label} (text): \"", end="")
    display_length = min(length, 40)
    for i in range(display_length):
        if 32 <= data[i] <= 126:  # Printable ASCII
            print(chr(data[i]), end="")
        else:
            print(".", end="")
    if length > 40:
        print("...\"")
    else:
        print("\"")

# Function to load a file into memory
def load_file(filename):
    try:
        with open(filename, 'rb') as f:
            data = f.read()
        return data
    except Exception as e:
        print(f"Error: Could not open file {filename}: {e}")
        return None

# Function to write data to a file
def write_file(filename, data):
    try:
        with open(filename, 'wb') as f:
            f.write(data)
        return True
    except Exception as e:
        print(f"Error: Could not write to file {filename}: {e}")
        return False

def main():
    # Try to import rdtsc extension
    try:
        import rdtsc
        print("RDTSC module loaded - using actual CPU cycle measurements")
    except ImportError:
        print("Warning: RDTSC module not available - using time-based approximation")
        print("For actual CPU cycle counts, build and install the rdtsc extension")
    
    # Input/output file names
    input_file = "test_input.txt"
    encrypted_file = "python_encrypted_output.bin"
    decrypted_file = "python_decrypted_output.txt"
    
    # Number of iterations for more accurate timing
    num_iterations = 10
    
    print("\n=== Python Multi-Key DEA Encryption Test ===\n")
    print(f"Input file: {input_file}")
    print(f"Number of iterations for encryption: {num_iterations}")
    
    # Initialize DEA
    dea = DEA()
    print("Setting up 4 encryption keys...")
    dea.reset()
    dea.set_key(0xAA)
    dea.set_key(0xBB)
    dea.set_key(0xCC)
    dea.set_key(0xDD)
    
    # Load the input file with timing
    print("Loading input file...")
    start_cycles = get_cycles()
    start_time = time.time()
    
    input_data = load_file(input_file)
    
    end_cycles = get_cycles()
    end_time = time.time()
    load_cycles = end_cycles - start_cycles
    load_time = (end_time - start_time) * 1000  # Convert to ms
    
    if input_data is None:
        print("Failed to load input file")
        return 1
    
    file_size = len(input_data)
    print(f"File loaded successfully: {file_size} bytes")
    print(f"File load time: {load_cycles:,} cycles ({load_time:.3f} ms)")
    
    print_data("Original (sample)", input_data)
    
    # Run a small encryption to warm up
    dea.reset()
    warm_up = dea.encrypt_block(input_data[:1024])
    
    # Start the encryption benchmark
    print(f"\nStarting encryption benchmark ({file_size} bytes × {num_iterations} iterations)...")
    
    encrypt_cycles = 0
    encrypt_time = 0
    
    # Multiple iterations for more accurate timing
    for j in range(num_iterations):
        dea.reset()
        start_cycles = get_cycles()
        start_time = time.time()
        
        encrypted = dea.encrypt_block(input_data)
        
        end_cycles = get_cycles()
        end_time = time.time()
        encrypt_cycles += (end_cycles - start_cycles)
        encrypt_time += (end_time - start_time) * 1000  # Convert to ms
    
    # Calculate average encryption time
    encrypt_cycles //= num_iterations
    encrypt_time /= num_iterations
    
    # Show a sample of the encrypted data
    print_data("Encrypted (sample)", encrypted)
    
    # Verify with decryption
    print("\nPerforming decryption...")
    dea.reset()
    
    start_cycles = get_cycles()
    start_time = time.time()
    
    decrypted = dea.decrypt_block(encrypted)
    
    end_cycles = get_cycles()
    end_time = time.time()
    decrypt_cycles = end_cycles - start_cycles
    decrypt_time = (end_time - start_time) * 1000  # Convert to ms
    
    print_data("Decrypted (sample)", decrypted)
    
    # Verify correctness
    if decrypted == input_data:
        print("\nVerification SUCCESSFUL - The decrypted text matches the original!")
    else:
        print("\nVerification FAILED - The decrypted text does not match the original!")
    
    # Write encrypted and decrypted data to files
    print("\nWriting output files...")
    start_cycles = get_cycles()
    start_time = time.time()
    
    write_success = True
    
    if write_file(encrypted_file, encrypted):
        print(f"Encrypted data written to {encrypted_file}")
    else:
        print("Failed to write encrypted data")
        write_success = False
    
    if write_file(decrypted_file, decrypted):
        print(f"Decrypted data written to {decrypted_file}")
    else:
        print("Failed to write decrypted data")
        write_success = False
    
    end_cycles = get_cycles()
    end_time = time.time()
    write_cycles = end_cycles - start_cycles
    write_time = (end_time - start_time) * 1000  # Convert to ms
    
    # Calculate total time
    total_cycles = load_cycles + encrypt_cycles + decrypt_cycles + write_cycles
    total_time = load_time + encrypt_time + decrypt_time + write_time
    
    # Print performance metrics
    mb_size = (file_size / (1024 * 1024))
    if mb_size < 1:
        size_str = f"{file_size / 1024:.2f}KB"
    else:
        size_str = f"{mb_size:.2f}MB"
    
    print(f"\n=== Performance Results ({size_str} file, {num_iterations} iterations) ===")
    print(f"File load:     {load_cycles:,} cycles ({load_time:.3f} ms) ({load_time/total_time*100:.3f}% of total)")
    print(f"Encryption:    {encrypt_cycles:,} cycles ({encrypt_time:.3f} ms) ({encrypt_time/total_time*100:.3f}% of total)")
    print(f"Decryption:    {decrypt_cycles:,} cycles ({decrypt_time:.3f} ms) ({decrypt_time/total_time*100:.3f}% of total)")
    print(f"File write:    {write_cycles:,} cycles ({write_time:.3f} ms) ({write_time/total_time*100:.3f}% of total)")
    print(f"Total:         {total_cycles:,} cycles ({total_time:.3f} ms)")
    
    # Calculate throughput
    encrypt_mb_per_second = (file_size / 1024.0 / 1024.0) / (encrypt_time / 1000.0)
    decrypt_mb_per_second = (file_size / 1024.0 / 1024.0) / (decrypt_time / 1000.0)
    
    # Calculate cycles per byte
    encrypt_cycles_per_byte = encrypt_cycles / file_size
    decrypt_cycles_per_byte = decrypt_cycles / file_size
    
    print("\nThroughput:")
    print(f"Encryption: {encrypt_mb_per_second:.2f} MB/second ({encrypt_cycles_per_byte:.2f} cycles/byte)")
    print(f"Decryption: {decrypt_mb_per_second:.2f} MB/second ({decrypt_cycles_per_byte:.2f} cycles/byte)")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())