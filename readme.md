View the youtube presentation in the link (in the "about" section) and the report for key findings and a more detailed analysis

# DEA (Data Encryption Algorithm) Implementation

**High-Performance Parallel Cryptography, MPI Distributed Computing, Hardware-Software Co-Design**

A C implementation showcasing **MPI parallel programming**, **distributed encryption**, and **FPGA-to-software migration** for multi-key XOR-based Data Encryption Algorithm (DEA). This explores CE/CS topics such as **parallel computing**, **cryptographic acceleration**, and **performance optimization**.

## Overview

This project demonstrates advanced **parallel programming techniques** and **distributed systems** concepts through a custom encryption algorithm that bridges **hardware implementation** (FPGA Verilog origins) with **high-performance software**. The implementation showcases:

- **Parallel Computing**: MPI-based distributed encryption with load balancing
- **Performance Engineering**: CPU cycle-accurate benchmarking and optimization
- **Hardware-Software Bridge**: Software implementation mirroring FPGA Verilog design patterns
- **Scalability Analysis**: Comprehensive parallel efficiency and speedup measurements
- **Systems Programming**: Low-level optimization with memory management and I/O efficiency
- **Computer Architecture**: Cache-aware design and CPU performance characterization

## Learning Objectives & Academic Value

- **Parallel Algorithms**: Understanding data decomposition and distributed processing
- **Performance Analysis**: Profiling, benchmarking, and optimization techniques
- **Concurrency**: MPI communication patterns and synchronization
- **Algorithm Design**: Cryptographic primitives and key management

- **Hardware-Software Co-Design**: Translating FPGA concepts to software
- **Computer Architecture**: CPU cycles, cache behavior, and memory hierarchy
- **Systems Optimization**: Low-level performance tuning and resource management
- **Embedded Systems**: Efficient algorithms suitable for hardware acceleration

**Comparison with FPGA Implementation:**
- **Throughput**: Software ~500 MB/s vs FPGA potential ~10+ GB/s
- **Latency**: Software variable vs FPGA deterministic cycle timing
- **Scalability**: Software scales with cores vs FPGA scales with logic resources
- **Power Efficiency**: FPGA advantage ~10-100x lower power per operation
- **Development Complexity**: Software easier to debug vs FPGA requires hardware expertise

## Features

- Multi-key XOR encryption with up to 4 rotating keys
- Chunk-based parallel processing with MPI
- CPU cycle-accurate performance measurement
- Support for files from bytes to gigabytes
- Automatic verification of encryption/decryption correctness
- Both binary and ASCII output formats
- Memory-efficient file processing

## File Structure

```
├── dea.h                    # DEA algorithm header
├── dea.c                    # DEA algorithm implementation
├── serial_dea.c             # Serial encryption program
├── mpi_dea.c               # MPI parallel encryption program
├── test_file_10b.c         # Generate 10-byte test file
├── test_file_100b.c        # Generate 100-byte test file
├── test_file_1kb.c         # Generate 1KB test file
├── test_file_1mb.c         # Generate 1MB test file
├── test_file_configurable.c # Generate configurable size test file
└── README.md               # This file
```

## Building the Project

### Prerequisites

- GCC or compatible C compiler
- MPI implementation (OpenMPI, MPICH, or Intel MPI)
- Windows: Microsoft MPI SDK
- Linux/macOS: OpenMPI or MPICH

### Compilation

#### Serial Version
```bash
gcc -o serial_dea serial_dea.c dea.c -O3
```

#### MPI Version
```bash
# Linux/macOS
mpicc -o mpi_dea mpi_dea.c dea.c -O3

# Windows with Microsoft MPI
gcc -o mpi_dea mpi_dea.c dea.c -I"C:\Program Files (x86)\Microsoft SDKs\MPI\Include" -L"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" -lmsmpi -O3
```

#### Test File Generators
```bash
gcc -o test_10b test_file_10b.c
gcc -o test_100b test_file_100b.c
gcc -o test_1kb test_file_1kb.c
gcc -o test_1mb test_file_1mb.c
gcc -o test_configurable test_file_configurable.c
```

## Usage

### Creating Test Files

```bash
# Generate different sized test files
./test_10b                           # Creates 10-byte file
./test_100b                          # Creates 100-byte file
./test_1kb                           # Creates 1KB file
./test_1mb                           # Creates 1MB file
./test_configurable                  # Creates 10MB file (default)
./test_configurable custom.txt 50   # Creates 50MB file named custom.txt
```

### Running Serial Encryption

```bash
./serial_dea
```

**Output files:**
- `serial_encrypted_output.bin` - Encrypted data as ASCII decimal values
- `serial_decrypted_output.txt` - Decrypted data (should match input)

### Running MPI Parallel Encryption

```bash
# Run with 4 processes
mpirun -np 4 ./mpi_dea

# Run with 8 processes
mpirun -np 8 ./mpi_dea
```

**Output files:**
- `encrypted_output.bin` - Encrypted data as ASCII decimal values
- `decrypted_output.txt` - Decrypted data (should match input)

## Algorithm Details

### DEA Encryption Process

1. **Key Setup**: Load up to 4 8-bit keys (0xAA, 0xBB, 0xCC, 0xDD by default)
2. **Key Rotation**: Use keys in rotating sequence for each byte
3. **XOR Encryption**: Each byte is XORed with the current active key
4. **Key Counter**: Automatically advances to next key after each byte

### Example Encryption
```
Input:  [A] [B] [C] [D] [E] ...
Keys:   AA  BB  CC  DD  AA  ...
Output: [A^AA] [B^BB] [C^CC] [D^DD] [E^AA] ...
```

### Parallel Processing Strategy

- **Data Chunking**: File divided evenly among MPI processes
- **Key Synchronization**: Each process starts with correct key offset
- **Independent Processing**: Each chunk encrypted independently
- **Result Assembly**: Master process collects and writes results

## Performance Benchmarking

The programs provide detailed performance metrics:

### Timing Breakdown
- **File Load Time**: Time to read input file
- **Encryption Time**: Core encryption performance (averaged over iterations)
- **Decryption Time**: Decryption verification
- **File Write Time**: Output file generation

### Performance Metrics
- **Throughput**: MB/s for encryption and decryption
- **Cycles per Byte**: CPU efficiency measurement
- **Scaling Analysis**: Parallel efficiency with multiple processes

### Sample Output
```
=== Performance Results (1MB file, 10 iterations) ===
File load:     1234567 cycles (0.411 ms) (5.2% of total)
Encryption:    5678901 cycles (1.893 ms) (71.8% of total)
Decryption:    987654 cycles (0.329 ms) (12.5% of total)
File write:    876543 cycles (0.292 ms) (11.1% of total)
Total:         7913579 cycles (2.638 ms)

Throughput:
Encryption:  526.34 MB/s
Decryption:  3035.67 MB/s

Cycles per byte:
Encryption:  5.40 cycles/byte
Decryption:  0.94 cycles/byte
```

## Configuration

### CPU Frequency Setting
Update the CPU frequency constant in both programs for accurate timing:
```c
const double CPU_FREQ_GHZ = 3.0; // Replace with your CPU frequency in GHz
```

### Default Settings
- **Input file**: `test_input.txt`
- **Encryption iterations**: 10 (for timing accuracy)
- **Keys**: 0xAA, 0xBB, 0xCC, 0xDD
- **Small file threshold**: 4 bytes (processed on master only)

## Optimization Features

### Small File Handling
- Files ≤ 4 bytes processed entirely on master process
- Avoids MPI overhead for tiny files

### Memory Efficiency
- Streaming file I/O for large files
- Chunked processing to minimize memory usage
- Buffer reuse across iterations

### Cache Warming
- Initial small encryption to warm CPU cache
- Multiple iterations for stable timing measurements

## Verification

Both programs automatically verify correctness by:
1. Encrypting the input data
2. Decrypting the encrypted data
3. Comparing decrypted result with original input
4. Reporting success/failure

## Troubleshooting

### Common Issues

1. **File not found**: Ensure `test_input.txt` exists or create using test generators
2. **MPI compilation errors**: Check MPI installation and include paths
3. **Performance inconsistency**: Ensure consistent CPU frequency and minimal system load
4. **Memory allocation failures**: Reduce file size or increase available memory

### Windows-Specific Notes
- Use Microsoft MPI SDK
- Adjust include and library paths in compilation commands
- Use `mpiexec` instead of `mpirun` if needed

### Linux/macOS Notes
- Install OpenMPI: `sudo apt-get install openmpi-bin openmpi-common libopenmpi-dev`
- Or MPICH: `sudo apt-get install mpich libmpich-dev`

## Performance Tips

1. **CPU Frequency**: Set correct frequency for accurate timing
2. **Process Count**: Use powers of 2 for optimal chunk distribution
3. **File Size**: Larger files show better parallel scaling
4. **System Load**: Run on dedicated/idle systems for consistent results
5. **Compiler Optimization**: Use -O3 flag for maximum performance

## License

This project is provided as-is for educational and research purposes.

## Contributing

Feel free to submit issues, feature requests, or improvements to enhance the implementation.
