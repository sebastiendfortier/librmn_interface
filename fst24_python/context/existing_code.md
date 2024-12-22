# Existing Code References

## 1. FSTPY Integration Points

```python
# Current FSTPY metadata handling
class FSTDataFrame(pd.DataFrame):
    """Extended DataFrame for FST metadata"""
    
    @classmethod
    def from_fst(cls, filename: str) -> 'FSTDataFrame':
        """Create DataFrame from FST file"""
        records = []
        with open_fst(filename) as f:
            for record in f:
                records.append({
                    'nomvar': record.nomvar,
                    'typvar': record.typvar,
                    'ni': record.ni,
                    'nj': record.nj,
                    'nk': record.nk,
                    'dateo': record.dateo,
                    'ip1': record.ip1,
                    'ip2': record.ip2,
                    'ip3': record.ip3,
                    'deet': record.deet,
                    'npas': record.npas,
                    'grtyp': record.grtyp,
                    'ig1': record.ig1,
                    'ig2': record.ig2,
                    'ig3': record.ig3,
                    'ig4': record.ig4
                })
        return cls(records)

# Data access patterns
def read_data(filename: str, **query):
    """Read data matching query"""
    with open_fst(filename) as f:
        records = f.matching_records(**query)
        return [r.data for r in records]
```

## 2. FSTD2NC Compatibility

```python
# Current coordinate handling
def get_coordinates(record):
    """Get lat/lon coordinates"""
    if record.grtyp in ['A', 'B', 'E', 'L']:
        return get_geographic_coords(record)
    elif record.grtyp in ['G', 'N', 'S']:
        return get_projected_coords(record)
    else:
        return get_custom_coords(record)

# Time handling
def decode_timestamp(dateo: int, npas: int, deet: int) -> datetime:
    """Convert FST timestamps to datetime"""
    base = datetime(1980, 1, 1) + timedelta(seconds=dateo)
    delta = timedelta(seconds=deet * npas)
    return base + delta
```

## 3. LibRMN Python Bindings

```python
# Current ctypes bindings
from ctypes import *

lib = CDLL('librmn.so')

# File operations
lib.c_fst_open.argtypes = [c_char_p, c_char_p]
lib.c_fst_open.restype = c_int

lib.c_fst_close.argtypes = [c_int]
lib.c_fst_close.restype = None

# Data access
lib.c_fst_read.argtypes = [c_int, POINTER(c_float), c_int, c_int, c_int]
lib.c_fst_read.restype = c_int

# Error handling
def check_error(code: int) -> None:
    """Check return code for errors"""
    if code < 0:
        msg = lib.c_fst_error_message(code)
        raise FSTError(msg.decode('utf-8'))
```

## 4. Performance Patterns

```python
# Current caching strategy
class RecordCache:
    """Cache for frequently accessed records"""
    def __init__(self, max_size: int = 1000):
        self._cache = {}
        self._max_size = max_size
        self._lock = threading.Lock()
    
    def get(self, key: str) -> Optional[np.ndarray]:
        with self._lock:
            return self._cache.get(key)
    
    def put(self, key: str, value: np.ndarray) -> None:
        with self._lock:
            if len(self._cache) >= self._max_size:
                self._cache.pop(next(iter(self._cache)))
            self._cache[key] = value

# Memory mapping
def map_large_file(filename: str) -> np.ndarray:
    """Memory map large files"""
    return np.memmap(filename, dtype='float32', mode='r',
                    shape=(ni, nj, nk), order='F')
```
