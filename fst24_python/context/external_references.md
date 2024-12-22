# External Project References

## 1. FSTPY
Repository: https://github.com/sebastiendfortier/fstpy

Key Features:
- Metadata handling via pandas DataFrame
- IP1/IP2/IP3 decoding
- Grid information processing
- Data type conversions

Example Usage:
```python
# Current FSTPY pattern
import fstpy
df = fstpy.StandardFileReader('myfile.std').to_pandas()
df = fstpy.compute_ip1_ip2(df)  # Convert encoded values
```

## 2. FSTD2NC
Repository: https://github.com/neishm/fstd2nc

Key Features:
- Direct xarray integration
- Efficient coordinate handling
- Grid transformations
- Time axis management

Example Usage:
```python
# Current FSTD2NC pattern
import fstd2nc.mixins
ds = fstd2nc.Dataset('myfile.std')
xr = ds.to_xarray()
```

## 3. Georef
Repository: https://gitlab.science.gc.ca/RPN-SI/georef

Key Features:
- Grid operations and transformations
- Coordinate system handling
- Interpolation capabilities
- Future integration target

Integration Points:
```python
# Future georef integration pattern
def get_grid_info(self) -> Dict:
    """Prepare grid info for georef"""
    return {
        'grid_type': self.grtyp,
        'parameters': [self.ig1, self.ig2, self.ig3, self.ig4],
        'dimensions': (self.ni, self.nj)
    }
```

## Integration Considerations

1. Common Data Patterns:
   - Metadata extraction
   - Grid information handling
   - Coordinate system management
   - Data type conversions

2. Performance Requirements:
   - Minimize data copies
   - Efficient metadata handling
   - Memory-mapped access where possible
   - Thread-safe operations

3. Compatibility Layer:
   - Support existing FSTPY DataFrame patterns
   - Match FSTD2NC xarray conventions
   - Prepare for georef integration
