# FST24 Python Interface Proposal

A modern Python interface for FST24 files, focusing on performance and ease of use. This proposal outlines a new approach to handling FST data with improved performance and memory efficiency.

## Key Features

- **Performance First**
  - Faster than fstpy for metadata operations (target: 2-3x speedup)
  - Matches fstd2nc's xarray conversion speed
  - Memory-efficient with support for large files

- **Modern Interface**
  - Native Polars DataFrame support
  - xarray integration with CF conventions
  - Thread-safe operations
  - Context manager support

- **Backend Support**
  - XDF and RSF formats via FST_OPTIONS
  - Memory mapping for large files (>1GB)
  - Parallel write support for RSF

## Usage Examples

```python
import fst24

# Fast metadata access (target: <0.05s)
with fst24.open("file.fst") as f:
    df = f.to_polars()  # Returns Polars DataFrame

# Efficient xarray conversion (target: <3.5s)
with fst24.open("file.fst") as f:
    ds = f.to_xarray(forecast_axis=True)  # Returns xarray Dataset

# Memory-efficient iteration
with fst24.open("file.fst") as f:
    for record in f.iter_records():
        # Lazy loading - data only read when accessed
        data = record.data  
```

## Performance Targets

1. **Metadata Operations**
   ```python
   # FST24 (target: 0.03s)
   df = fst24.read_metadata("file.fst")
   
   # Current fstpy (0.10s)
   df = fstpy.StandardFileReader("file.fst").to_pandas()
   ```

2. **XArray Conversion**
   ```python
   # FST24 (target: 3.5s)
   ds = fst24.to_xarray("file.fst", forecast_axis=True)
   
   # Current fstd2nc (3.5s)
   ds = fstd2nc.Buffer("file.fst", forecast_axis=True).to_xarray()
   ```

## Design Decisions

1. **Memory Management**
   - Memory mapping for files >1GB
   - Zero-copy views when possible
   - Proper cleanup in destructors
   - Global memory limit configuration

2. **Thread Safety**
   - File-level locking for RSF backend
   - Thread-local storage for handles
   - Atomic write operations

3. **Data Organization**
   - Run-level metadata caching
   - Batch record processing
   - Lazy data loading

4. **Error Handling**
   - Clear error messages
   - Proper resource cleanup
   - Safe fallbacks for memory issues

## Implementation Plan

### Phase 1: Core (1-2 days)
- Basic file operations
- Record handling
- Thread safety
- Memory management

### Phase 2: Data Access (2-3 days)
- Memory mapping
- Array views
- Performance optimization
- Resource management

### Phase 3: Integration (2-3 days)
- Polars integration
- XArray integration
- CF conventions
- Performance tuning

### Phase 4: Testing (2-3 days)
- Unit tests
- Integration tests
- Benchmarks
- Documentation

## Dependencies

### Core
- numpy>=1.24.0
- polars>=0.20.0
- xarray>=2023.12.0
- typing_extensions>=4.8.0

### System
- librmn>=20.0.0 (with FST24/RSF support)

## Configuration

### Backend Selection
```bash
export FST_OPTIONS="BACKEND=RSF"  # or XDF
```

### Memory Management
```python
import fst24
fst24.set_memory_limit(16 * 1024**3)  # 16GB limit
```

## Questions for Review

1. Are the performance targets sufficient for your use cases?
2. Do you need any additional metadata fields beyond the core set?
3. Are there specific grid types or operations that need optimization?
4. Would you prefer different default behaviors for memory management?
5. Are there additional integration points needed beyond Polars and xarray?

## Next Steps

1. Review and feedback on the proposed design
2. Prioritization of features and performance targets
3. Discussion of specific use cases and requirements
4. Agreement on implementation timeline
