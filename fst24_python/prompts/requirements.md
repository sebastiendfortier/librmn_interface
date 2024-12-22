# FST24 Python Interface Requirements

## Overview
This project creates a modern Python interface for the FST24 file format, focusing on:
- High performance data access
- Pythonic API design
- Integration with modern data science tools (polars, xarray)
- No Python version dependencies

## Core Requirements

1. Backend Support:
   - XDF (legacy) and RSF (new) formats
   - Environment variable configuration: FST_OPTIONS="BACKEND=RSF"
   - Thread-safe operations
   - Parallel write capabilities (RSF only)

2. Data Access:
   - Direct memory access via numpy arrays
   - F_CONTIGUOUS array support
   - Memory mapping for large files
   - Lazy loading capabilities

3. Metadata Handling:
   - Fast extraction using ctypes
   - Batch processing support
   - Polars DataFrame as primary container
   - Optional pandas conversion

4. Integration Points:
   - xarray for scientific data
   - libgeoref for grid operations
   - fstpy/fstd2nc compatibility

## Performance Goals
- Minimize memory allocations
- Efficient bulk operations
- Thread-safe implementations
- Cache-friendly data structures
