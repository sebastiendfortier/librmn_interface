# Design Decisions

## 1. Implementation Order

1. Core File Interface
   - Direct librmn bindings using ctypes
   - Backend selection (RSF/XDF) via environment variables
   - Thread safety per backend
   - Error propagation and cleanup

2. Metadata Handling
   - Core Metadata (Fast Path):
     ```python
     @dataclass
     class CoreMetadata:
         """Fast-path metadata extraction"""
         nomvar: str
         typvar: str
         etiket: str
         datev: int
         ip1: int
         ip2: int
         ip3: int
         ni: int
         nj: int
         nk: int
         grtyp: str
         ig1: int
         ig2: int
         ig3: int
         ig4: int
     ```
   - Extended Metadata (Optional):
     ```python
     class MetadataDecoder:
         """Optional metadata decoding"""
         def decode_metadata(self, core: CoreMetadata) -> Dict[str, Any]:
             """Decode additional metadata fields"""
             return {
                 'bounded': self._check_bounded(core),
                 'missing_data': self._check_missing(core),
                 # ... other decoded fields
             }
     ```
   - Performance Focus:
     - Zero-copy core metadata
     - Lazy evaluation for extended fields
     - Batch processing optimization

3. Data Access Layer
   - Memory mapping for large files
   - F-contiguous array handling
   - Safe memory views
   - Lazy loading

4. Integration Layer
   - xarray with CF conventions (match fstd2nc speed)
   - Grid operations placeholder
   - Performance benchmarking

## 2. Core API Structure

### Direct librmn Integration
```python
# ctypes definitions for direct memory access
class FSTRecord(Structure):
    """Direct mapping to fst_record_struct"""
    _fields_ = [
        ('dateo', c_int32),
        ('datev', c_int32),
        ('deet', c_int32),
        ('npas', c_int32),
        ('ni', c_int32),
        ('nj', c_int32),
        ('nk', c_int32),
        ('ip1', c_int32),
        ('ip2', c_int32),
        ('ip3', c_int32),
        ('typvar', c_char * 2),
        ('nomvar', c_char * 4),
        ('etiket', c_char * 12),
        ('grtyp', c_char * 1),
        ('ig1', c_int32),
        ('ig2', c_int32),
        ('ig3', c_int32),
        ('ig4', c_int32),
        ('datyp', c_int32),
        ('nbits', c_int32)
    ]

class Fst24File:
    """Direct librmn file interface with minimal overhead"""
    def __init__(self, path: str, mode: str = 'r'):
        self._path = path.encode('utf-8')
        self._mode = mode.encode('utf-8')
        self._handle = None
        self._lib = CDLL('librmn.so')
        self._setup_c_bindings()
        
    def _setup_c_bindings(self):
        """Setup direct C function bindings"""
        # File operations
        self._lib.c_fst24_open.argtypes = [c_char_p, c_char_p]
        self._lib.c_fst24_open.restype = c_int32
        self._lib.c_fst24_close.argtypes = [c_int32]
        
        # Data access
        self._lib.c_fst24_getk.argtypes = [c_int32, c_void_p, POINTER(c_int32), 
                                          POINTER(c_int32), POINTER(c_int32)]
        self._lib.c_fst24_getk.restype = c_int32
        
    def read_array(self, record: FSTRecord) -> np.ndarray:
        """Zero-copy array read when possible"""
        shape = (record.ni, record.nj, record.nk)
        dtype = self._get_numpy_dtype(record.datyp)
        
        # Try direct memory view first
        try:
            data = self._get_direct_view(shape, dtype)
            if data is not None:
                return data
        except:
            pass
            
        # Fallback to copy if needed
        return self._read_with_copy(shape, dtype)

class SafeArrayView:
    """Safe wrapper for array views with memory management"""
    def __init__(self, ptr: int, shape: Tuple[int, ...], dtype: np.dtype):
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
                    (self._dtype._type_ * np.prod(self._shape)).from_address(self._ptr),
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
                self._view = np.array(self._ptr, dtype=self._dtype, copy=True, order='F')
        return self._view
        
    def __del__(self):
        """Cleanup to prevent memory leaks"""
        self._view = None
        self._original = None

class Fst24Record:
    """Minimal wrapper around FST record with safe memory handling"""
    def __init__(self, handle: int, record: FSTRecord):
        self._handle = handle
        self._record = record
        self._data_view = None
        
    @property
    def data(self) -> np.ndarray:
        """Lazy load with safe memory handling"""
        if self._data_view is None:
            ptr = self._get_direct_pointer()
            shape = self._get_shape()
            dtype = self._get_dtype()
            self._data_view = SafeArrayView(ptr, shape, dtype)
        return self._data_view.get_view()

class Fst24DataInterface:
    """Optimized data interface with minimal copying"""
    def __init__(self, records: List[Fst24Record], 
                 decode_metadata: bool = False,
                 decoder: Optional[MetadataDecoder] = None):
        self._records = records
        self._decode_metadata = decode_metadata
        self._decoder = decoder or DefaultMetadataDecoder()
        
    def to_polars(self) -> pl.LazyFrame:
        """Metadata-only Polars conversion"""
        # Fast path for core metadata
        df = pl.LazyFrame({
            'nomvar': [bytes(r._record.nomvar).strip() for r in self._records],
            'typvar': [bytes(r._record.typvar).strip() for r in self._records],
            'etiket': [bytes(r._record.etiket).strip() for r in self._records],
            'datev': [r._record.datev for r in self._records],
            'ip1': [r._record.ip1 for r in self._records],
            'ip2': [r._record.ip2 for r in self._records],
            'ip3': [r._record.ip3 for r in self._records],
            'grid_info': [self._grid_info(r._record) for r in self._records]
        })
        
        # Optional extended metadata
        if self._decode_metadata:
            extended = [self._decoder.decode_metadata(r) for r in self._records]
            df = df.with_columns([
                pl.col(k).alias(k) for k in extended[0].keys()
            ])
            
        return df
        
    def to_xarray(self, forecast_axis: bool = False) -> xr.Dataset:
        """Efficient xarray conversion with CF conventions from fstd2nc"""
        ds = xr.Dataset()
        
        for record in self._records:
            var_name = bytes(record._record.nomvar).strip().decode('utf-8')
            coords = self._get_fstd2nc_coordinates(record)
            
            if forecast_axis:
                coords['forecast'] = self._get_forecast_coord(record)
                
            ds[var_name] = xr.DataArray(
                data=record.data,
                dims=list(coords.keys()),
                coords=coords,
                attrs=self._get_fstd2nc_attributes(record)
            )
            
        return ds
```

## 3. Memory Management

### Thread Safety Requirements
- RSF Backend:
  - All operations are thread-safe
  - Parallel write support
  - No explicit locking needed
- XDF Backend:
  - Limited thread safety
  - File-level locking required
  - No parallel write support

### Memory Mapping Strategy
- Files > 1GB: Automatic memory mapping
- Files ≤ 1GB: Direct memory access
- Cleanup on context exit

### Error Handling
- Direct error code propagation
- Resource cleanup in __exit__
- Clear Python exceptions

## 4. Performance Targets

### Metadata Operations
- Core metadata (no decode): Sub-0.03s for typical files (179 records)
- Extended metadata (with decode): Sub-0.12s
- Batch processing: Scale linearly with record count
- Metadata decoding should be strictly opt-in

### Data Access
- Zero-copy when possible
- F-contiguous array handling
- Memory mapping for files > 1GB

### xarray Integration
- Target conversion time: Sub-3.5s for typical files
- Efficient forecast axis support
- Preserve CF conventions
- Handle IP3 values gracefully

### Parallel Operations
- Thread-safe file handles
- Respect 998 file limit
- Efficient run-level caching

### Benchmark Files
```python
# Test file paths for consistent benchmarking
BENCHMARK_FILES = [
    '/space/hall5/sitestore/eccc/prod/hubs/gridpt/dbase/prog/glbdiag.usr/2024122200_000',  # Core metadata
    '/space/hall5/sitestore/eccc/prod/hubs/gridpt/dbase/prog/glbdiag.usr/2024122200_001',  # Extended metadata
    '/space/hall5/sitestore/eccc/prod/hubs/gridpt/dbase/prog/glbdiag.usr/2024122200_002'   # xarray conversion
]

# Metadata performance (target: match or exceed fstpy)
def benchmark_metadata():
    """Benchmark metadata extraction speed"""
    with Fst24File(BENCHMARK_FILES[0]) as f:
        # Core metadata (target: < 0.03s)
        t0 = time.time()
        df = f.to_polars()
        core_time = time.time() - t0

        # Extended metadata (target: < 0.12s)
        t0 = time.time()
        df = f.to_polars(decode_metadata=True)
        extended_time = time.time() - t0

        return {'core': core_time, 'extended': extended_time}

# xarray performance (target: match fstd2nc)
def benchmark_xarray():
    """Benchmark xarray conversion speed"""
    with Fst24File(BENCHMARK_FILES[2]) as f:
        t0 = time.time()
        ds = f.to_xarray(
            forecast_axis=True,
            rpnstd_metadata=True
        )
        return time.time() - t0  # Target: < 3.5s
```

### Environment Setup
```bash
# Required for metadata decoding
export CMCCONST=/home/smco502/datafiles/constants

# Backend selection
export FST_OPTIONS="BACKEND=RSF"
```
