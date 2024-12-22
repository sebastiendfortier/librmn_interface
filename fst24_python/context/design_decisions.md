# Design Decisions

## 1. Core API Structure

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
    def __init__(self, records: List[Fst24Record]):
        self._records = records
        
    def to_polars(self) -> pl.LazyFrame:
        """Metadata-only Polars conversion"""
        return pl.LazyFrame({
            'nomvar': [bytes(r._record.nomvar).strip() for r in self._records],
            'typvar': [bytes(r._record.typvar).strip() for r in self._records],
            'etiket': [bytes(r._record.etiket).strip() for r in self._records],
            'datev': [r._record.datev for r in self._records],
            'ip1': [r._record.ip1 for r in self._records],
            'ip2': [r._record.ip2 for r in self._records],
            'ip3': [r._record.ip3 for r in self._records],
            'grid_info': [self._grid_info(r._record) for r in self._records]
        })
        
    def to_xarray(self, forecast_axis: bool = False) -> xr.Dataset:
        """Efficient xarray conversion with CF conventions from fstd2nc"""
        ds = xr.Dataset()
        
        for record in self._records:
            var_name = bytes(record._record.nomvar).strip().decode('utf-8')
            # Use fstd2nc's CF convention mapping
            coords = self._get_fstd2nc_coordinates(record)
            
            if forecast_axis:
                coords['forecast'] = self._get_forecast_coord(record)
                
            # Create view of data when possible
            ds[var_name] = xr.DataArray(
                data=record.data,  # Uses lazy loading with potential zero-copy
                dims=list(coords.keys()),
                coords=coords,
                attrs=self._get_fstd2nc_attributes(record)
            )
            
        return ds
```

## 2. Memory Management

### Zero-Copy Strategy
```python
def get_array_view(ptr: int, shape: Tuple[int, ...], dtype: np.dtype) -> np.ndarray:
    """Create numpy view of C/Fortran memory"""
    try:
        # Attempt zero-copy view
        return np.ctypeslib.as_array(
            (dtype._type_ * np.prod(shape)).from_address(ptr),
            shape=shape
        )
    except:
        # Fallback to copy if needed
        return np.array(ptr, dtype=dtype, copy=True, order='F')
```

## 3. Performance Considerations

### Memory Efficiency
- Use direct librmn memory access when possible
- Minimize data copying between Python and C/Fortran
- Lazy loading of record data
- Metadata-only operations where possible

### Thread Safety
- Use native librmn thread safety mechanisms
- Minimal Python-level locking
- Respect 998 file limit

### Error Handling
- Propagate librmn errors directly
- Clean error messages for Python users
- Proper resource cleanup

## 4. Virtual Zarr Integration

### FST Virtual Store
```python
class FSTZarrStore(MutableMapping):
    """Virtual Zarr store backed by FST files"""
    def __init__(self, fst_path: str, chunk_shape: Optional[Tuple[int, ...]] = None):
        self._fst_file = Fst24File(fst_path)
        self._chunk_shape = chunk_shape
        self._metadata = None
        
    def _init_metadata(self):
        """Initialize Zarr metadata from FST structure"""
        if self._metadata is None:
            with self._fst_file as f:
                records = f.read_records()
                self._metadata = self._create_zarr_metadata(records)
                
    def _create_zarr_metadata(self, records: List[Fst24Record]) -> Dict:
        """Create Zarr metadata from FST records"""
        # Map FST structure to Zarr chunks
        # Use natural FST record boundaries where possible
        return {
            'zarr_version': 2,
            'shape': self._compute_shape(records),
            'chunks': self._chunk_shape or self._compute_optimal_chunks(records),
            'dtype': self._get_dtype(records),
            'compressor': None,  # Direct access to FST compression
            'fill_value': None,
            'order': 'F'  # Match FST's Fortran order
        }
        
    def __getitem__(self, key: str) -> bytes:
        """Map Zarr chunk requests to FST records"""
        if key == '.zarray':
            self._init_metadata()
            return json.dumps(self._metadata).encode()
            
        # Parse chunk coordinates from key
        chunk_coords = self._parse_chunk_key(key)
        return self._read_chunk(chunk_coords)
        
    def _read_chunk(self, coords: Tuple[int, ...]) -> bytes:
        """Read chunk data directly from FST records"""
        with self._fst_file as f:
            # Map chunk coordinates to FST records
            records = self._find_records_for_chunk(coords)
            # Efficient assembly of chunk data
            return self._assemble_chunk(records, coords)

class FSTZarrArray:
    """High-level interface for FST files as Zarr arrays"""
    def __init__(self, fst_path: str):
        self._store = FSTZarrStore(fst_path)
        self._zarr_array = zarr.Array(self._store)
        
    @property
    def zarr_array(self) -> zarr.Array:
        """Access as standard Zarr array"""
        return self._zarr_array
        
    def to_dask(self) -> da.Array:
        """Convert to Dask array for distributed computing"""
        return da.from_zarr(self._store)
```

## 5. OGC API Support

### Web Map Service Interface
```python
class FSTWebMapService:
    """Base class for OGC-compliant web services"""
    def __init__(self, fst_path: str):
        self._fst_file = Fst24File(fst_path)
        self._cache = LRUCache(maxsize=1000)  # Cache common requests
        
    def get_capabilities(self) -> Dict:
        """Generate WMS capabilities document"""
        with self._fst_file as f:
            metadata = f.read_metadata()
            return self._create_capabilities(metadata)
            
    def get_map(self, bbox: Tuple[float, float, float, float],
                width: int, height: int, layers: List[str]) -> np.ndarray:
        """Efficient map generation from FST data"""
        cache_key = self._make_cache_key(bbox, width, height, layers)
        
        # Check cache first
        if cache_key in self._cache:
            return self._cache[cache_key]
            
        # Generate map efficiently
        with self._fst_file as f:
            records = f.read_records(variables=layers)
            map_data = self._generate_map(records, bbox, width, height)
            self._cache[cache_key] = map_data
            return map_data
            
    def _generate_map(self, records: List[Fst24Record],
                     bbox: Tuple[float, float, float, float],
                     width: int, height: int) -> np.ndarray:
        """Efficient map generation with minimal memory usage"""
        # Use record metadata to optimize data access
        required_records = self._find_required_records(records, bbox)
        
        # Generate map with minimal memory overhead
        return self._render_map(required_records, bbox, width, height)
```

## 6. Performance Optimizations

### Zarr Integration
- Use FST record boundaries for natural chunking
- Direct access to FST compression
- Minimal data copying for Zarr views

### Web Service Optimization
- LRU caching for common requests
- Efficient coordinate transformations
- Memory-aware rendering
- Parallel request handling

### Memory Management
- Shared memory views where possible
- Efficient coordinate transformations
- Smart caching strategies
