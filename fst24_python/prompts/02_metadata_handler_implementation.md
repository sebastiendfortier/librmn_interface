# Metadata Handler Implementation

## 1. Core Metadata Classes

```python
from dataclasses import dataclass
from typing import Dict, Optional, Union, List
from ctypes import Structure, c_int, c_char, POINTER
import polars as pl
import numpy as np

@dataclass
class Fst24Metadata:
    """Container for FST record metadata"""
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

    @classmethod
    def from_c_struct(cls, c_meta: Structure) -> 'Fst24Metadata':
        """Create metadata from C structure"""
        return cls(
            nomvar=c_meta.nomvar.decode().strip(),
            typvar=c_meta.typvar.decode().strip(),
            dateo=c_meta.dateo,
            datev=c_meta.datev,
            deet=c_meta.deet,
            npas=c_meta.npas,
            ni=c_meta.ni,
            nj=c_meta.nj,
            nk=c_meta.nk,
            ip1=c_meta.ip1,
            ip2=c_meta.ip2,
            ip3=c_meta.ip3,
            etiket=c_meta.etiket.decode().strip(),
            grtyp=c_meta.grtyp.decode().strip(),
            ig1=c_meta.ig1,
            ig2=c_meta.ig2,
            ig3=c_meta.ig3,
            ig4=c_meta.ig4
        )

    def to_dict(self) -> Dict[str, Union[str, int]]:
        """Convert metadata to dictionary"""
        return {
            'nomvar': self.nomvar,
            'typvar': self.typvar,
            'dateo': self.dateo,
            'datev': self.datev,
            'deet': self.deet,
            'npas': self.npas,
            'ni': self.ni,
            'nj': self.nj,
            'nk': self.nk,
            'ip1': self.ip1,
            'ip2': self.ip2,
            'ip3': self.ip3,
            'etiket': self.etiket,
            'grtyp': self.grtyp,
            'ig1': self.ig1,
            'ig2': self.ig2,
            'ig3': self.ig3,
            'ig4': self.ig4
        }

    def to_polars(self) -> pl.DataFrame:
        """Convert metadata to polars DataFrame"""
        return pl.DataFrame([self.to_dict()])
```

## 2. Test Cases

```python
def test_metadata_creation():
    c_meta = create_test_metadata()  # Helper function
    meta = Fst24Metadata.from_c_struct(c_meta)
    assert meta.nomvar == "TT"
    assert meta.typvar == "P"
    assert meta.ni == 100

def test_metadata_conversion():
    meta = create_test_metadata()
    df = meta.to_polars()
    assert isinstance(df, pl.DataFrame)
    assert "nomvar" in df.columns
    assert df.shape == (1, 18)  # All metadata fields
```

## 3. Implementation Notes

1. Memory Management:
   - Efficient string handling
   - Proper cleanup of C resources
   - Cache-friendly data structures

2. Integration:
   - Support FSTPY patterns
   - Match existing interfaces
   - Prepare for future extensions

3. Performance:
   - Minimize allocations
   - Batch processing support
   - Thread-safe operations
