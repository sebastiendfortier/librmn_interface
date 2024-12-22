# Record Implementation

## Requirements

1. Core Record Class:
```python
from dataclasses import dataclass
from typing import Optional, Dict, Union
import numpy as np
import polars as pl
import xarray as xr
from ctypes import Structure, c_int, c_char, c_void_p

@dataclass
class Fst24Record:
    """FST record implementation with data access and conversion support"""
    # Metadata fields from C structure
    nomvar: str
    typvar: str
    dateo: int
    datev: int
    deet: int
    npas: int
    ni: int
    nj: int
    nk: int
    ip1: int
    ip2: int
    ip3: int
    etiket: str
    grtyp: str
    ig1: int
    ig2: int
    ig3: int
    ig4: int
    
    # Internal state
    _data_ptr: Optional[c_void_p] = None
    _file_handle: Optional[int] = None
    
    @classmethod
    def from_c_record(cls, c_record: Structure) -> 'Fst24Record':
        """Create record from C structure"""
        return cls(
            nomvar=c_record.nomvar.decode().strip(),
            typvar=c_record.typvar.decode().strip(),
            # ... other field conversions
        )
    
    def get_data(self) -> np.ndarray:
        """Get record data as numpy array"""
        if self._data_ptr is None:
            self._load_data()
        return np.array(self._data_ptr, 
                       dtype=self._get_numpy_dtype(),
                       shape=(self.ni, self.nj, self.nk),
                       order='F')
    
    def to_polars(self) -> pl.DataFrame:
        """Convert to polars DataFrame"""
        return pl.DataFrame({
            'data': self.get_data().ravel(),
            **self._get_metadata_dict()
        })
    
    def to_xarray(self) -> xr.DataArray:
        """Convert to xarray DataArray"""
        return xr.DataArray(
            self.get_data(),
            coords=self._get_coordinates(),
            attrs=self._get_metadata_dict()
        )
```

2. Test Cases:
```python
def test_record_creation():
    c_record = create_test_record()  # Helper function
    record = Fst24Record.from_c_record(c_record)
    assert record.nomvar == "TT"
    assert record.ni == 100

def test_data_access():
    record = create_test_record()
    data = record.get_data()
    assert isinstance(data, np.ndarray)
    assert data.shape == (record.ni, record.nj, record.nk)
    assert data.flags['F_CONTIGUOUS']
```

## Implementation Notes

1. Memory Management:
   - Lazy loading of data
   - Proper cleanup of C resources
   - Support for large arrays

2. Type Conversions:
   - Handle all FST data types
   - Preserve metadata in conversions
   - Support compression/packing

3. Integration:
   - Follow polars/xarray best practices
   - Maintain compatibility with existing code
   - Support future georef integration
