# Data Access Layer Implementation

## 1. Requirements

### Memory Management
- Memory mapping for files > 1GB
- Zero-copy array views when possible
- F-contiguous array handling
- Safe resource cleanup

### Performance
- Lazy loading of data
- Efficient batch operations
- Thread-safe access
- Minimal data copying

## 2. Implementation Steps

1. **Array View Management**
   ```python
   class SafeArrayView:
       """Safe wrapper for array views with memory management"""
       def __init__(self, ptr: int, shape: Tuple[int, ...], 
                    dtype: np.dtype):
           self._ptr = ptr
           self._shape = shape
           self._dtype = dtype
           self._view = None
           self._original = None  # Keep reference to prevent GC
           
       def get_view(self, copy_on_write: bool = True) -> np.ndarray:
           """Get array view with safety checks"""
           if self._view is None:
               try:
                   # Attempt zero-copy view
                   arr = np.ctypeslib.as_array(
                       (self._dtype._type_ * np.prod(self._shape))
                           .from_address(self._ptr),
                       shape=self._shape
                   )
                   # Ensure F-contiguous for Fortran compatibility
                   if not arr.flags.f_contiguous:
                       arr = np.asfortranarray(arr)
                   
                   # Make copy if mutation is possible
                   if copy_on_write:
                       self._view = arr.copy()
                   else:
                       self._view = arr
                       self._original = arr  # Prevent GC
               except:
                   # Fallback to copy
                   self._view = np.array(self._ptr, dtype=self._dtype,
                                       copy=True, order='F')
           return self._view
           
       def __del__(self):
           """Cleanup to prevent memory leaks"""
           self._view = None
           self._original = None
   ```

2. **Memory Mapping**
   ```python
   class FileMapper:
       """Memory mapping manager"""
       def __init__(self, path: str, threshold: int = 1024**3):
           self._path = path
           self._threshold = threshold
           self._mmap = None
           
       def should_map(self) -> bool:
           """Check if file should be memory mapped"""
           return os.path.getsize(self._path) > self._threshold
           
       def get_array(self, offset: int, shape: Tuple[int, ...],
                     dtype: np.dtype) -> np.ndarray:
           """Get array from file"""
           if self.should_map():
               if self._mmap is None:
                   self._mmap = np.memmap(self._path, mode='r',
                                        dtype=dtype, order='F')
               return np.asfortranarray(
                   self._mmap[offset:offset + np.prod(shape)]
                       .reshape(shape, order='F')
               )
           else:
               # Direct read for small files
               return np.fromfile(self._path, dtype=dtype,
                                count=np.prod(shape),
                                offset=offset).reshape(shape, order='F')
   ```

3. **Data Access Interface**
   ```python
   class DataAccess:
       """High-level data access interface"""
       def __init__(self, handle: int, record: FSTRecord):
           self._handle = handle
           self._record = record
           self._data_view = None
           self._mapper = None
           
       @property
       def data(self) -> np.ndarray:
           """Lazy load data with memory management"""
           if self._data_view is None:
               shape = (self._record.ni,
                       self._record.nj,
                       self._record.nk)
               dtype = self._get_numpy_dtype()
               
               if self._should_use_direct_view():
                   # Try zero-copy view
                   ptr = self._get_direct_pointer()
                   self._data_view = SafeArrayView(ptr, shape, dtype)
               else:
                   # Use memory mapping or direct read
                   if self._mapper is None:
                       self._mapper = FileMapper(self._get_file_path())
                   offset = self._get_data_offset()
                   return self._mapper.get_array(offset, shape, dtype)
                   
           return self._data_view.get_view()
           
       def _get_numpy_dtype(self) -> np.dtype:
           """Map FST data type to numpy dtype"""
           datyp = self._record.datyp
           if datyp == 1:  # FST_TYPE_FLOAT
               return np.dtype(np.float32)
           elif datyp == 2:  # FST_TYPE_DOUBLE
               return np.dtype(np.float64)
           elif datyp == 3:  # FST_TYPE_INT
               return np.dtype(np.int32)
           # ... other type mappings
   ```

## 3. Testing Requirements

1. **Memory Management**
   ```python
   def test_memory_mapping():
       # Large file (>1GB)
       with Fst24File("large.fst") as f:
           record = f.read_records()[0]
           data = record.data  # Should use memory mapping
           assert isinstance(data, np.ndarray)
           assert data.flags.f_contiguous
           
       # Small file
       with Fst24File("small.fst") as f:
           record = f.read_records()[0]
           data = record.data  # Should use direct read
           assert isinstance(data, np.ndarray)
           assert data.flags.f_contiguous
   ```

2. **Array Views**
   ```python
   def test_array_views():
       view = SafeArrayView(ptr, (100, 100), np.float32)
       data = view.get_view()
       assert data.flags.f_contiguous
       assert not data.flags.owndata  # Zero-copy view
       
       data = view.get_view(copy_on_write=True)
       assert data.flags.owndata  # Copy made
   ```

3. **Thread Safety**
   ```python
   def test_concurrent_access():
       file = Fst24File("test.fst")
       records = file.read_records()
       
       def access_data(record):
           return record.data
           
       with ThreadPoolExecutor() as ex:
           results = list(ex.map(access_data, records))
   ```

## 4. Performance Requirements

1. **Memory Mapping**
   - Activation: Files > 1GB
   - Overhead: < 1ms setup time
   - Memory usage: Proportional to accessed data

2. **Array Views**
   - Zero-copy success rate: > 90%
   - F-contiguous conversion: < 1ms
   - View creation: < 0.1ms

3. **Data Access**
   - First access: < 10ms
   - Subsequent access: < 1ms
   - Memory cleanup: Immediate

## 5. Error Handling

1. **Memory Mapping**
   - File size limits
   - Permission errors
   - Memory allocation failures

2. **Array Views**
   - Invalid pointers
   - Shape mismatches
   - Type conversion errors

3. **Resource Management**
   - Memory leaks prevention
   - File handle cleanup
   - Thread safety violations
