

# Data Access Implementation

## 1. Core Data Access Class

```python
from typing import Optional, Dict, Union, Tuple
import numpy as np
import xarray as xr
from ctypes import c_void_p, c_int, POINTER, cast

class DataAccessManager:
    """Manages data access and conversions for FST records"""
    
    def __init__(self, file_handle: int):
        self._handle = file_handle
        self._data_cache: Dict[int, np.ndarray] = {}
        self._setup_c_bindings()
    
    def _setup_c_bindings(self) -> None:
        """Configure ctypes bindings for data access"""
        self._lib.c_fst24_get_data.argtypes = [c_int, c_void_p, POINTER(c_int)]
        self._lib.c_fst24_get_data.restype = c_int
        
    def get_data(self, record_id: int, shape: Tuple[int, ...], 
                 dtype: np.dtype) -> np.ndarray:
        """Get record data as numpy array
        
        Args:
            record_id: Unique record identifier
            shape: Array dimensions (ni, nj, nk)
            dtype: Numpy data type
        """
        if record_id in self._data_cache:
            return self._data_cache[record_id]
            
        data_ptr = c_void_p()
        dims = (c_int * 3)(*shape)
        status = self._lib.c_fst24_get_data(self._handle, data_ptr, dims)
        
        if status < 0:
            raise FSTIOError("Failed to read data")
            
        array = np.frombuffer(
            cast(data_ptr, POINTER(dtype._type_)),
            dtype=dtype,
            count=np.prod(shape)
        ).reshape(shape, order='F')
        
        self._data_cache[record_id] = array
        return array
```

## 2. Test Cases

```python
def test_data_access():
    manager = DataAccessManager(file_handle)
    data = manager.get_data(
        record_id=1,
        shape=(10, 10, 1),
        dtype=np.float32
    )
    assert isinstance(data, np.ndarray)
    assert data.flags['F_CONTIGUOUS']
    assert data.shape == (10, 10, 1)

def test_data_caching():
    manager = DataAccessManager(file_handle)
    data1 = manager.get_data(1, (10, 10, 1), np.float32)
    data2 = manager.get_data(1, (10, 10, 1), np.float32)
    assert data1 is data2  # Same object (cached)
```

## 3. Implementation Notes

1. Memory Management:
   - Use memory mapping for large files
   - Implement LRU cache for frequently accessed data
   - Proper cleanup of C resources

2. Performance:
   - Direct memory access via numpy
   - Minimize data copies
   - Thread-safe caching

3. Integration:
   - Support for all FST data types
   - Coordinate system handling
   - Grid transformations
