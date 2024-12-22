# Type Definitions Implementation

## Requirements

1. Core Protocol Classes:
   ```python
   from typing import Protocol, Dict, Union, List, Tuple, Iterator, Optional
   from dataclasses import dataclass
   import numpy as np
   import polars as pl
   import xarray as xr

   class Fst24Record(Protocol):
       """Core record interface"""
       # Metadata fields
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

       def get_data(self) -> np.ndarray: ...
       def to_polars(self) -> pl.DataFrame: ...
       def to_xarray(self) -> xr.DataArray: ...

   class Fst24Query(Protocol):
       """Query interface"""
       def __iter__(self) -> Iterator[Fst24Record]: ...
       def __next__(self) -> Optional[Fst24Record]: ...
       def filter(self, **kwargs) -> 'Fst24Query': ...
   ```

2. Type Aliases:
   ```python
   # Common type definitions
   QueryParams = Dict[str, Union[str, int, float]]
   GridInfo = Dict[str, Union[str, List[int], Tuple[int, int]]]
   MetadataDict = Dict[str, Union[str, int]]
   DataArray = Union[np.ndarray, xr.DataArray]
   ```

3. Error Types:
   ```python
   class FSTError(Exception):
       """Base exception for FST operations"""
       pass

   class FSTIOError(FSTError):
       """File I/O related errors"""
       pass

   class FSTMetadataError(FSTError):
       """Metadata handling errors"""
       pass
   ```

## Implementation Guidelines

1. Use these types consistently across the codebase
2. Implement proper type checking where needed
3. Ensure compatibility with existing interfaces
4. Document any deviations from protocols

## Test Requirements

1. Type checking tests
2. Protocol compliance tests
3. Error handling tests
