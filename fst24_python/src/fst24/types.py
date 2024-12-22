from typing import TypeVar, Protocol, Dict, Iterator, Optional
from dataclasses import dataclass
import numpy as np
import polars as pl
import xarray as xr

class Fst24Record(Protocol):
    """Protocol defining the interface for FST records"""
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
