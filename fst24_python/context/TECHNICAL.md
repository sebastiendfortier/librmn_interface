# FST24 Technical Reference

## 1. Core FST API

### File Operations
```c
// Open/close operations
int c_fst24_open(char *filename, char *mode);  // Returns file handle
int c_fst24_close(int handle);                 // Cleanup resources

// Record operations
int c_fst24_find(int handle, void *where);     // Position to record
int c_fst24_getk(int handle, void *buffer, int *ni, int *nj, int *nk);  // Read data
int c_fst24_putk(int handle, void *buffer, int ni, int nj, int nk);     // Write data

// Metadata operations
int c_fst24_get_meta(int handle, fst_record_struct *meta);  // Get record info
int c_fst24_put_meta(int handle, fst_record_struct *meta);  // Set record info
```

### Error Handling
- Return values < 0 indicate errors
- Thread-safe error codes and messages
- Resource cleanup on error

## 2. Metadata Fields

### Core Fields
```python
@dataclass
class CoreMetadata:
    """Essential record metadata"""
    nomvar: str    # Variable name (4 chars)
    typvar: str    # Type of field (2 chars)
    etiket: str    # Label (12 chars)
    datev: int     # Valid date stamp
    ip1: int       # Level encoding
    ip2: int       # Time encoding
    ip3: int       # User defined
    ni: int        # First dimension
    nj: int        # Second dimension
    nk: int        # Third dimension
    grtyp: str     # Grid type (1 char)
    datyp: int     # Data type code
    nbits: int     # Bits per value
```

### Extended Fields
```python
@dataclass
class ExtendedMetadata:
    """Optional metadata fields"""
    bounded: bool      # Has bounds
    missing: bool      # Has missing values
    compressed: bool   # Is compressed
    level_type: str   # Level type description
    ip_kind: int      # IP coding kind
    ip_level: float   # Decoded level value
```

## 3. Grid Types

### Support Requirements
- All grid types must be supported from initial release
- Typical model run contains 1-2 grid types
- No pre-generation of coordinates needed
- Cache only current grid descriptors

### Geographic (lat/lon)
- `'A'`: Global lat-lon
- `'B'`: Lat-lon window
- `'L'`: Polar stereographic
- `'E'`: Geographic window

### Projected
- `'N'`: North polar stereographic
- `'S'`: South polar stereographic
- `'G'`: Gaussian grid

### Special
- `'U'`: Unstructured grid
- `'X'`: Custom/experimental
- `'Y'`: Generic grid (user defined)

### Grid Caching Strategy
```python
class GridCache:
    """Minimal grid descriptor cache"""
    def __init__(self):
        self._descriptors = {}  # Typically 1-2 entries
        self._current = None    # Most recent grid
        
    def get_descriptor(self, grtyp: str, ig1: int, ig2: int,
                      ig3: int, ig4: int) -> int:
        """Get cached grid descriptor or create new one"""
        key = (grtyp, ig1, ig2, ig3, ig4)
        if key not in self._descriptors:
            # Limit cache size, remove oldest if needed
            if len(self._descriptors) >= 2:
                oldest = next(iter(self._descriptors))
                del self._descriptors[oldest]
            # Create new descriptor
            self._descriptors[key] = self._create_descriptor(*key)
        self._current = key
        return self._descriptors[key]
```

## 4. Memory Management

### File Mapping
```python
class FileMapper:
    """Memory mapping for large files"""
    def __init__(self, path: str, threshold: int = 1024**3):
        self._path = path
        self._threshold = threshold  # Default: 1GB
        self._mmap = None
    
    def get_array(self, offset: int, shape: Tuple[int, ...],
                  dtype: np.dtype) -> np.ndarray:
        """Get array view or copy"""
        if self.should_map():
            return self._get_mapped_array(offset, shape, dtype)
        else:
            return self._get_direct_array(offset, shape, dtype)
```

### Array Views
```python
class SafeArrayView:
    """Thread-safe array views"""
    def __init__(self, ptr: int, shape: Tuple[int, ...],
                 dtype: np.dtype, copy: bool = True):
        self._ptr = ptr
        self._shape = shape
        self._dtype = dtype
        self._copy = copy
        self._view = None
    
    def get_view(self) -> np.ndarray:
        """Get array view with safety checks"""
        if self._view is None:
            self._view = self._create_view()
        return self._view.copy() if self._copy else self._view
```

## 5. Thread Safety

### File Locking
```python
class FileLock:
    """File-level locking for RSF backend"""
    def __init__(self, path: str):
        self._path = path
        self._lock = threading.Lock()
        self._handle = None
    
    def __enter__(self):
        """Acquire lock and open file"""
        self._lock.acquire()
        self._handle = c_fst24_open(self._path.encode(), b'r')
        return self._handle
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Release lock and close file"""
        if self._handle is not None:
            c_fst24_close(self._handle)
        self._lock.release()
```

### Handle Management
```python
class HandleManager:
    """Thread-local handle storage"""
    def __init__(self):
        self._handles = threading.local()
        
    def get_handle(self, path: str) -> int:
        """Get thread-local handle"""
        if not hasattr(self._handles, path):
            handle = c_fst24_open(path.encode(), b'r')
            setattr(self._handles, path, handle)
        return getattr(self._handles, path)
```

## 6. Performance Considerations

### Metadata Caching
- Cache at run level (same forecast hour)
- Share between threads when possible
- Clear on memory pressure

### Batch Operations
- Pre-allocate arrays for multiple records
- Process metadata in batches
- Share coordinate systems

### Memory Limits
- Default: 16GB per process
- Configurable via API
- Automatic cleanup on threshold

## 7. Error Codes

### Core Operations
- `-1`: General error
- `-2`: File not found
- `-3`: Permission denied
- `-4`: Invalid handle
- `-5`: Memory error

### Data Access
- `-10`: Record not found
- `-11`: Invalid dimensions
- `-12`: Type mismatch
- `-13`: Decode error
- `-14`: Grid error

### Thread Safety
- `-20`: Lock error
- `-21`: Thread limit
- `-22`: Handle error
- `-23`: Cleanup error

## 8. Multi-File Handling

### Polars Integration
```python
def read_multiple_files(paths: List[str]) -> pl.DataFrame:
    """Read multiple FST files into single DataFrame"""
    dfs = []
    for path in paths:
        with Fst24File(path) as f:
            dfs.append(f.to_polars())
    return pl.concat(dfs)
```

### XArray Integration
```python
def open_multiple_files(paths: List[str], 
                       forecast_axis: bool = True) -> xr.Dataset:
    """Open multiple FST files as single dataset"""
    datasets = []
    for path in paths:
        with Fst24File(path) as f:
            datasets.append(f.to_xarray(forecast_axis=forecast_axis))
    return xr.merge(datasets)
```

## 9. Time Management

### Time Handling Strategy
- Use CMC standard format only
- Convert to/from Python datetime for external interfaces
- All times in UTC
- No special calendar support initially

### Time Calculations
```python
def calculate_forecast_hour(deet: int, npas: int) -> float:
    """Calculate forecast hour from deet and npas
    
    Args:
        deet: Time step in seconds
        npas: Number of time steps
        
    Returns:
        Forecast hour as float
    """
    return (deet * npas) / 3600.0

def datetime_to_cmc(dt: datetime) -> int:
    """Convert Python datetime to CMC stamp (UTC)"""
    if dt.tzinfo is not None:
        dt = dt.astimezone(timezone.utc)
    return c_fst24_date_to_stamp(
        dt.year, dt.month, dt.day,
        dt.hour, dt.minute, dt.second
    )

def cmc_to_datetime(stamp: int) -> datetime:
    """Convert CMC stamp to Python datetime (UTC)"""
    y, m, d, h, mn, s = c_fst24_stamp_to_date(stamp)
    return datetime(y, m, d, h, mn, s, tzinfo=timezone.utc)
```

### Time Fields
- `date`: Model run date (source time)
- `datev`: Validity date of the data
- `deet`: Time step in seconds
- `npas`: Number of time steps
- `ip2`: Usually forecast hour (but check encoding)

## 10. IP Management

### Using convip_plus
```python
# Import from librmn
convip_plus = lib.ConvipPlus
convip_plus.argtypes = [
    POINTER(c_float),  # Output value
    POINTER(c_int32),  # Input IP
    POINTER(c_int32),  # Kind
    c_int32,          # Mode (encode=0, decode=1)
]

def decode_ip(ip: int, default_kind: int = 2) -> Tuple[float, int]:
    """Decode IP using convip_plus
    
    Args:
        ip: IP value to decode
        default_kind: Default kind if not encoded in IP
        
    Returns:
        (value, kind) tuple
    """
    value = c_float()
    kind = c_int32(default_kind)
    mode = c_int32(1)  # decode mode
    
    status = convip_plus(
        byref(value),
        byref(c_int32(ip)),
        byref(kind),
        mode
    )
    
    if status < 0:
        raise ValueError(f"Failed to decode IP: {ip}")
        
    return float(value.value), int(kind.value)

def encode_ip(value: float, kind: int) -> int:
    """Encode value and kind into IP
    
    Args:
        value: Value to encode
        kind: Kind of level/coordinate
        
    Returns:
        Encoded IP value
    """
    ip = c_int32()
    mode = c_int32(0)  # encode mode
    
    status = convip_plus(
        byref(c_float(value)),
        byref(ip),
        byref(c_int32(kind)),
        mode
    )
    
    if status < 0:
        raise ValueError(f"Failed to encode IP: {value}, {kind}")
        
    return int(ip.value)
```

### IP Kinds
Common values from fstpy:
- `0`: Height (m)
- `1`: Sigma
- `2`: Pressure (mb)
- `3`: Arbitrary code
- `4`: Height (M)
- `5`: Hybrid coordinates
- `6`: Theta
- `10`: Hours
- `15`: Meters (reserved for special levels)

### Notes
- IP values are 32-bit integers
- Upper 8 bits encode kind
- Lower 24 bits encode value
- Some special values don't follow this pattern
- Always use convip_plus for encoding/decoding
- Check fstpy's implementation for special cases

## 11. Implementation Notes

### Multi-File Operations
- Support opening multiple files simultaneously
- Handle file limits (998 max from librmn)
- Consider memory impact of concurrent files
- Implement proper cleanup

### Time Management
- Keep time fields coherent
- Validate time relationships
- Consider timezone handling
- Support different time units

### IP Handling
- Use convip_plus for all IP operations
- Cache decoded values when possible
- Handle special level values
- Validate IP consistency

### Reference Code
- Check fstpy's dataframe.py and dataframe_utils.py
- Improve on their patterns where possible
- Focus on clean, maintainable implementation
- Consider performance implications

## 12. C Integration Guidelines

### Batch Metadata Reading
```c
// Target: c_fst24_read_meta_batch
// Guidelines:
// - Use OpenMP for parallel reading
// - Pre-allocate metadata array in caller
// - Return actual number of records read
// - Handle partial success cases
// - Support filtering criteria
// Implementation notes:
// - Check OMP_NUM_THREADS for parallelism
// - Use thread-local storage for handles
// - Minimize memory allocation
// - Consider cache line alignment
```

### Grid Coordinate Generation
```c
// Target: c_fst24_get_grid_coords
// Guidelines:
// - Reuse georef's ezscint functions
// - Support all grid types (A,B,E,L,N,S,G)
// - Pre-allocate coordinate arrays
// - Handle bounds for CF compliance
// Implementation notes:
// - Check grid validity before calculation
// - Cache grid descriptors
// - Support lazy coordinate generation
// - Handle missing/invalid coordinates
```

### Time Decoding
```c
// Target: c_fst24_decode_time
// Guidelines:
// - Handle all CMC time formats
// - Support forecast hour calculation
// - Validate field coherence
// - Handle timezone conversions
// Implementation notes:
// - Use existing newdate/difdatr
// - Handle special datev values
// - Support different time units
// - Maintain backwards compatibility
```

### IP Processing
```c
// Target: c_fst24_decode_ip_batch
// Guidelines:
// - Extend existing convip_plus
// - Support batch processing
// - Handle all IP encodings
// - Cache common values
// Implementation notes:
// - Use lookup tables for speed
// - Handle special IP values
// - Support custom decodings
// - Maintain thread safety
```

## 13. Performance Optimization Points

### Metadata Operations
- Batch read records in C (target: 1000 records/ms)
- Cache metadata at run level
- Use thread-local storage for handles
- Pre-allocate arrays for batch operations

### Grid Operations
- Cache grid descriptors by type
- Lazy coordinate generation
- Share coordinates between similar grids
- Use SIMD for coordinate calculations where available

### Memory Management
- Use mmap for files > 1GB
- Pre-allocate buffers for batch operations
- Share memory between similar operations
- Clear caches under memory pressure

### Thread Safety
- Use OpenMP for C-level parallelism
- Implement file-level locking for RSF
- Use thread-local storage for handles
- Support concurrent file access

## 14. Implementation Order

### Phase 1: Core C Functions
1. Batch metadata reading
2. Grid coordinate generation
3. Time field handling
4. IP batch processing

### Phase 2: Python Integration
1. ctypes bindings
2. Memory management
3. Error handling
4. Thread safety

### Phase 3: High-Level Features
1. Polars integration
2. XArray support
3. CF conventions
4. Performance tuning

### Phase 4: Optimization
1. Caching strategies
2. Memory mapping
3. Batch operations
4. Thread safety

## 15. Critical Considerations

### Thread Safety
- RSF backend requires file locking
- XDF backend is thread-safe
- Handle limit of 998 per process
- Use thread-local storage

### Memory Management
- Pre-allocate for batch operations
- Use mmap for large files
- Clear caches under pressure
- Handle allocation failures

### Error Handling
- Propagate C errors to Python
- Clean up resources on failure
- Provide detailed error messages
- Support partial success

### Performance
- Target metadata: < 0.05s for 1000 records
- Target data: < 10ms first access
- Target coordinates: < 100ms generation
- Target memory: < 2x data size

## Memory Management

### File Size Optimization
- Optimize for typical 2GB files
- Support larger files with FST24
- Use memory mapping for files > 1GB
- Monitor total memory usage

### Caching Strategy
```python
class IPCache:
    """Minimal IP value cache"""
    def __init__(self):
        # Typically ~100 values per IP type
        self._ip1_cache = LRUCache(maxsize=100)
        self._ip2_cache = LRUCache(maxsize=100)
        self._ip3_cache = LRUCache(maxsize=100)
        
    def get_decoded_ip(self, ip: int, ip_type: int) -> Tuple[float, int]:
        """Get decoded IP from cache or decode"""
        cache = self._get_cache(ip_type)
        if ip not in cache:
            cache[ip] = decode_ip(ip)
        return cache[ip]
```

## 16. Common Operations Examples

### Basic File Operations
```python
# Single file reading
with fst24.open("model_run.fst") as f:
    # Get all TT (temperature) records
    temps = f.search(nomvar="TT")
    
    # Get surface pressure at hour 24
    sfc_press = f.search(nomvar="P0", ip2=24)
    
    # Get all levels for specific variable
    all_levels = f.search(nomvar="HU", ip2=0)

# Multiple file handling
files = ["run_00.fst", "run_12.fst"]
with fst24.open_many(files) as runs:
    # Combine into single dataset
    ds = runs.to_xarray()
```

### XArray Integration
```python
# Convert to xarray with CF conventions
ds = fst24.to_xarray("model.fst", forecast_axis=True)

# Export to other formats
ds.to_netcdf("output.nc")  # NetCDF export
ds.rio.to_raster("output.tiff")  # GeoTIFF via rioxarray

# AI/ML preparation
features = ds[["TT", "HU", "UU", "VV"]].to_array()
features = features.transpose("time", "level", "y", "x", "variable")

# Visualization with xarray
ds.TT.isel(level=0).plot()  # Basic plot
ds.HU.plot.contourf()       # Contour plot

# QGIS/GIS integration
ds.rio.write_crs("EPSG:4326")  # Set projection
ds.rio.to_raster("for_qgis.tiff")
```

### Batch Processing
```python
# Process multiple runs
runs = fst24.glob("*/model.fst")
with fst24.ProcessPool() as pool:
    datasets = pool.map(fst24.to_xarray, runs)
    
# Combine forecasts
ds = xr.concat(datasets, dim="ensemble")

# Data cataloging
cat = fst24.create_catalog(runs)
cat.to_zarr("catalog.zarr")  # Efficient storage
```

## 17. Future Extensions

### Data Format Support
- Direct GRIB2 conversion without NetCDF intermediate
- Zarr format support for cloud storage
- Dask integration for larger-than-memory operations
- Arrow/Parquet support for columnar storage

### AI/ML Integration
- Direct tensor conversion (PyTorch, TensorFlow)
- Feature preprocessing pipelines
- Training data generation utilities
- Model input/output handlers

### Visualization
- Built-in plotting utilities
- Interactive dashboard support
- WebGL-based viewers
- Real-time streaming support

### Cloud Integration
- S3/Azure/GCP storage support
- Distributed processing
- Serverless deployment options
- Container-friendly configurations

### Performance Enhancements
- GPU-accelerated coordinate transformations
- Vectorized IP decoding
- Parallel file writing
- Streaming record processing

### GIS Integration
- Direct GDAL/OGR support
- Vector data handling
- Projection utilities
- OGC services support

### Data Catalog Features
- Metadata indexing
- Search capabilities
- Version tracking
- Provenance tracking

### Operational Features
- Real-time monitoring
- Health checks
- Performance metrics
- Resource usage tracking

## 18. Format Conversion

### Bidirectional Conversion
```python
# FST -> Other formats
with fst24.open("model.fst") as f:
    ds = f.to_xarray()
    # Export to various formats
    ds.to_netcdf("output.nc")
    ds.rio.to_raster("output.tiff")
    ds.to_zarr("output.zarr")

# Other formats -> FST
ds = xr.open_dataset("input.nc")
fst24.from_xarray(ds, "output.fst")

ds = xr.open_zarr("input.zarr")
fst24.from_xarray(ds, "output.fst")

ds = xr.open_rasterio("input.tiff")
fst24.from_xarray(ds, "output.fst")
```

### Format Mapping Rules
```python
class FormatConverter:
    """Handle bidirectional format conversion"""
    def __init__(self):
        self._cf_to_fst = {
            # Standard CF names to FST nomvar
            'air_temperature': 'TT',
            'specific_humidity': 'HU',
            'eastward_wind': 'UU',
            'northward_wind': 'VV',
            'surface_air_pressure': 'P0'
        }
        self._fst_to_cf = {v: k for k, v in self._cf_to_fst.items()}
        
    def to_fst(self, ds: xr.Dataset, output: str):
        """Convert xarray Dataset to FST format
        
        Handles:
        - CF convention mapping
        - Grid type detection
        - Level encoding
        - Time encoding
        - Missing value handling
        """
        with fst24.create(output) as f:
            for name, da in ds.data_vars.items():
                # Get FST metadata
                nomvar = self._cf_to_fst.get(da.attrs.get('standard_name'), name)
                grid_info = self._get_grid_info(da)
                level_info = self._get_level_info(da)
                time_info = self._get_time_info(da)
                
                # Write to FST
                f.write(
                    data=da.values,
                    nomvar=nomvar,
                    grid_type=grid_info.type,
                    ig1=grid_info.ig1,
                    ig2=grid_info.ig2,
                    ig3=grid_info.ig3,
                    ig4=grid_info.ig4,
                    ip1=level_info.ip1,
                    ip2=time_info.ip2,
                    ip3=0  # Default
                )
```

### Supported Conversions
- NetCDF (via xarray)
  - Full CF convention support
  - Coordinate system preservation
  - Metadata mapping
  - Missing value handling

- GeoTIFF (via rioxarray)
  - Projection preservation
  - Resolution handling
  - Band mapping
  - Metadata conversion

- Zarr (via xarray)
  - Chunking preservation
  - Compression handling
  - Metadata mapping
  - Dimension alignment

- GRIB2 (via cfgrib/xarray)
  - Single layer export only
  - One variable/level/forecast time per GRIB2 message
  - No combined/stacked fields
  - Example:
    ```python
    # Export individual layers to GRIB2
    with fst24.open("model.fst") as f:
        ds = f.to_xarray()
        
        # Export each layer separately
        for var in ds.data_vars:
            da = ds[var]
            # For each forecast hour
            for t in da.time:
                # For each level
                for level in da.level:
                    # Extract single layer
                    layer = da.sel(time=t, level=level)
                    # Export to GRIB2
                    layer.to_grib2(
                        f"{var}_level{level.item()}_hour{t.item()}.grib2"
                    )
    ```

### Conversion Considerations
- Maintain data precision
- Preserve coordinate systems
- Handle missing values
- Map metadata fields
- Validate data ranges
- Check dimension compatibility
- Support compression options
- Handle large files efficiently

### Special Cases
```python
class SpecialCaseHandler:
    """Handle special conversion cases"""
    def handle_rotated_grid(self, da: xr.DataArray) -> GridInfo:
        """Convert rotated lat-lon to FST grid"""
        # Extract rotation parameters
        lon_pole = da.attrs.get('grid_north_pole_longitude')
        lat_pole = da.attrs.get('grid_north_pole_latitude')
        
        # Convert to FST grid type 'L'
        return GridInfo(
            type='L',
            ig1=self._encode_angle(lat_pole),
            ig2=self._encode_angle(lon_pole),
            ig3=0,
            ig4=0
        )
        
    def handle_hybrid_levels(self, da: xr.DataArray) -> LevelInfo:
        """Convert hybrid levels to FST encoding"""
        # Get coefficients
        a_coef = da.attrs.get('atmosphere_hybrid_sigma_pressure_a_coefficient')
        b_coef = da.attrs.get('atmosphere_hybrid_sigma_pressure_b_coefficient')
        
        # Encode as hybrid coordinate (kind=5)
        return LevelInfo(
            ip1=encode_ip(a_coef, kind=5),
            ip2=encode_ip(b_coef, kind=5)
        )
```
``` 
