Data Encryption Accelerator (DEA) - Quick Guide
A simple encryption system that cycles through multiple keys to encrypt data, implemented in both C and Verilog.
What it does

Takes 4 different encryption keys
Cycles through keys as it encrypts each byte
Supports single-thread and parallel processing
Uses simple XOR encryption (easy to extend)

Files
dea.h/dea.c - Core encryption engine
serial_dea.c - Basic single-thread example
parallel_dea.c - Simulated parallel version
mpi_dea.c - MPI parallel version
Build It (Windows)
bash# Basic version
gcc -o serial_dea serial_dea.c dea.c

# Parallel simulation (no MPI needed)

gcc -o parallel_dea.exe parallel_dea.c dea.c

# MPI version (needs MS-MPI installed)

# First make sure your mpi_dea.c includes mpi.h: #include <mpi.h>

gcc -I"C:\Program Files (x86)\Microsoft SDKs\MPI\Include" ^
-L"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" ^
-o mpi_dea.exe mpi_dea.c dea.c -lmsmpi
Run It
bash# Basic version

gcc -L"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" -o mpi_dea.exe mpi_dea.c dea.c -lmsmpi

.\serial_dea.exe

# Simulated parallel

.\parallel_dea.exe

# MPI version (use 4 processes)

mpiexec -n 4 .\mpi_dea.exe
How It Works

Set 4 keys - Load 4 different 8-bit keys
Encrypt - XOR each byte with current key, then move to next key
Cycle keys - After 4th key, go back to 1st key
Decrypt - Same process (XOR is reversible)
