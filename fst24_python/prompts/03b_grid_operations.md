Based on the implementation order from composer_instructions.md (lines 45-61), let's continue with the grid operations implementation:

```markdown:fst24_python/prompts/03b_grid_operations.md
# Grid Operations Implementation

## 1. Core Grid Handler Class
```python
from typing import Dict, Tuple, Optional, Union
import numpy as np
from ctypes import c_int, c_float, POINTER, Structure

class GridHandler:
    """Handles grid operations and coordinate transformations"""
    
    def __init__(self, file_handle: int):
        self._handle = file_handle
        self._coord_cache: Dict[int, Tuple[np.ndarray, np.ndarray]] = {}
        self._setup_c_bindings()
    
    def _setup_c_bindings(self) -> None:
        """Configure ctypes bindings for grid operations"""
        self._lib.c_fst24_get_grid_desc.argtypes = [c_int, POINTER(Structure)]
        self._lib.c_fst24_get_grid_desc.restype = c_int
        self._lib.c_fst24_get_coords.argtypes = [c_int, c_void_p, c_void_p]
        self._lib.c_fst24_get_coords.restype = c_int
    
    def get_coordinates(self, record_id: int, 
                       shape: Tuple[int, int]) -> Tuple[np.ndarray, np.ndarray]:
        """Get latitude/longitude coordinates for grid
        
        Args:
            record_id: Unique record identifier
            shape: Grid dimensions (ni, nj)
        """
        if record_id in self._coord_cache:
            return self._coord_cache[record_id]
            
        ni, nj = shape
        lat = np.empty((ni, nj), dtype=np.float32, order='F')
        lon = np.empty((ni, nj), dtype=np.float32, order='F')
        
        status = self._lib.c_fst24_get_coords(
            self._handle,
            lat.ctypes.data_as(POINTER(c_float)),
            lon.ctypes.data_as(POINTER(c_float))
        )
        
        if status < 0:
            raise FSTIOError("Failed to get coordinates")
            
        coords = (lat, lon)
        self._coord_cache[record_id] = coords
        return coords
    
    def get_grid_info(self) -> Dict[str, Union[str, int]]:
        """Get grid description and parameters"""
        grid_desc = Structure()  # Define proper grid structure
        status = self._lib.c_fst24_get_grid_desc(self._handle, grid_desc)
        
        if status < 0:
            raise FSTIOError("Failed to get grid description")
            
        return {
            'grtyp': grid_desc.grtyp.decode(),
            'ig1': grid_desc.ig1,
            'ig2': grid_desc.ig2,
            'ig3': grid_desc.ig3,
            'ig4': grid_desc.ig4
        }
```

## 2. Test Cases
```python
def test_coordinate_generation():
    handler = GridHandler(file_handle)
    lat, lon = handler.get_coordinates(1, (10, 10))
    assert isinstance(lat, np.ndarray)
    assert isinstance(lon, np.ndarray)
    assert lat.shape == (10, 10)
    assert lon.shape == (10, 10)
    assert lat.flags['F_CONTIGUOUS']
    assert lon.flags['F_CONTIGUOUS']

def test_grid_info():
    handler = GridHandler(file_handle)
    info = handler.get_grid_info()
    assert 'grtyp' in info
    assert all(f'ig{i}' in info for i in range(1, 5))
```

## 3. Implementation Notes

1. Memory Management:
   - Efficient coordinate caching
   - Memory mapping for large grids
   - Proper cleanup of resources

2. Performance:
   - Direct memory access
   - Minimize coordinate recalculations
   - Thread-safe operations

3. Integration:
   - Support for all grid types
   - Future georef compatibility
   - Coordinate system transformations
