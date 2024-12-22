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

def validate_time_coherence(record: FSTRecord) -> bool:
    """Validate time field coherence
    
    Checks:
    - datev consistency with date + forecast
    - ip2 consistency with forecast hour
    - deet/npas consistency
    """
    forecast_hour = calculate_forecast_hour(record.deet, record.npas)
    expected_datev = record.date + timedelta(hours=forecast_hour)
    return (
        record.datev == expected_datev and
        record.ip2 == int(forecast_hour)
    )
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
``` 
