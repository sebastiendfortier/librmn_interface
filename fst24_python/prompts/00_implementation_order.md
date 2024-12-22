# Implementation Order

## Phase 1: Core Interface (1-2 days)

1. **Core File Interface**
   - Basic file operations (open, close)
   - Record reading and writing
   - Error handling and resource management
   - Thread safety implementation

2. **Metadata Handling**
   - Core metadata extraction
   - Extended metadata support
   - Efficient caching strategies
   - IP decoding methods

## Phase 2: Data Access (2-3 days)

3. **Data Access Layer**
   - Memory mapping implementation
   - Array view management
   - F-contiguous array handling
   - Resource cleanup

4. **Performance Optimization**
   - Batch operations
   - Memory usage monitoring
   - Thread safety improvements
   - Cache optimization

## Phase 3: Integration (2-3 days)

5. **Polars Integration**
   - Schema definition
   - Batch record processing
   - DataFrame interface
   - Performance tuning

6. **XArray Integration**
   - Coordinate system management
   - CF convention handling
   - Dataset creation
   - Performance optimization

## Phase 4: Testing and Documentation (2-3 days)

7. **Testing**
   - Unit tests for all components
   - Integration tests
   - Performance benchmarks
   - Memory leak checks

8. **Documentation**
   - API documentation
   - Usage examples
   - Performance guidelines
   - Error handling guide

## Performance Targets

1. **Core Operations**
   - File open: < 1ms
   - Record read: < 0.1ms per record
   - Memory overhead: < 1.2x data size

2. **Data Access**
   - First access: < 10ms
   - Subsequent access: < 1ms
   - Memory mapping setup: < 1ms

3. **Polars Integration**
   - DataFrame creation: < 0.05s
   - Memory overhead: < 1.5x metadata size
   - Batch processing: < 0.1ms per record

4. **XArray Integration**
   - Dataset creation: < 3.5s
   - Memory overhead: < 2x data size
   - Coordinate generation: < 100ms

## Dependencies

1. **Core Dependencies**
   - numpy>=1.24.0
   - polars>=0.20.0
   - xarray>=2023.12.0
   - typing_extensions>=4.8.0

2. **Development Dependencies**
   - pytest>=7.4.0
   - pytest-benchmark>=4.0.0
   - pytest-cov>=4.1.0
   - black>=23.12.0
   - mypy>=1.7.0

## Environment Setup

1. **Backend Selection**
   ```bash
   export FST_OPTIONS="BACKEND=RSF"  # or XDF
   ```

2. **Memory Limits**
   ```python
   import fst24
   fst24.set_memory_limit(16 * 1024**3)  # 16GB limit
   ```

3. **Thread Safety**
   ```python
   import fst24
   fst24.enable_thread_safety()  # Enable thread-safe operations
   ```

## Implementation Notes

1. **Memory Management**
   - Use memory mapping for files > 1GB
   - Implement proper cleanup in destructors
   - Monitor memory usage in batch operations

2. **Thread Safety**
   - Use file-level locking for RSF backend
   - Implement thread-local storage for handles
   - Ensure atomic write operations

3. **Performance**
   - Cache metadata at run level
   - Use zero-copy views when possible
   - Implement batch processing for records

4. **Error Handling**
   - Use custom exception hierarchy
   - Implement proper cleanup on errors
   - Provide detailed error messages 
