## Performance Metrics

The benchmark measures and reports:

File load time (in cycles and milliseconds)
Encryption time (in cycles and milliseconds)
Decryption time (in cycles and milliseconds)
File write time (in cycles and milliseconds)
Total execution time
Throughput (MB/second)
Cycles per byte for encryption and decryption

## DEA Algorithm

The DEA (Data Encryption Algorithm) implemented here uses XOR operations with a rotating set of keys. Key features:

Supports up to 4 different 8-bit keys
Keys are cycled through sequentially during encryption/decryption
XOR-based encryption means the decryption process is identical to encryption
Simple and efficient implementation optimized for Python

## Output Files

The benchmark generates two output files:

python_encrypted_output.bin - The encrypted version of the input file
python_decrypted_output.txt - The decrypted data (should match the original input)

## CPU Cycle Measurement

The benchmark uses one of several methods to measure CPU cycles:

Pure Python high-resolution timing (default)
Windows QueryPerformanceCounter when available
(Optional) psutil process timing when installed

Note: These are approximations of actual CPU cycles, scaled based on the CPU frequency.

## Comparison with C Implementation

For complete performance analysis, compare these results with the C and MPI implementations:

The Python implementation is typically slower due to interpreter overhead
The C implementation provides a baseline for single-threaded performance
The MPI implementation demonstrates parallel processing benefits

Customization
Adjust these parameters in the benchmark script to match your requirements:

num_iterations - Number of encryption iterations for accurate timing
CPU_FREQ_GHZ - Your CPU frequency in GHz (for cycle estimation)
Input/output filenames

License
