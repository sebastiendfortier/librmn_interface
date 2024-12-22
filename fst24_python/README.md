# FST24 Python Interface

A high-performance Python interface for FST24 files, designed to replace the Python functionality in both fstpy and fstd2nc.

## Design Philosophy

1. **Direct librmn Integration**
   - Zero-copy memory access when possible
   - Direct mapping to FST structures
   - Minimal overhead between Python and C/Fortran

2. **Memory Efficiency**
   - Lazy loading of data
   - Safe memory views with proper cleanup
   - Metadata-only operations where possible
   - Support for large files within 250GB limit

3. **Performance Focus**
   - Aim for 2x performance vs fstpy
   - Efficient parallel access across runs
   - Thread-safe operations
   - Respect for 998 file limit

## Key Components

### 1. Core File Interface
```python
with Fst24File("path/to/file.fst") as f:
    # Thread-safe file operations
    records = f.read_records()
    
    # Efficient data access
    for record in records:
        # Lazy loading with zero-copy when possible
        data = record.data
```

### 2. Data Interfaces

#### Polars Integration (Primary)
```python
# Metadata-focused DataFrame
df = records.to_polars()  # Returns LazyFrame for efficiency
```

#### XArray Integration
```python
# With CF conventions from fstd2nc
ds = records.to_xarray(forecast_axis=True)
```

## Implementation Details

1. **Memory Safety**
   - Safe array views with proper cleanup
   - Copy-on-write for mutable operations
   - Automatic F-contiguous array handling
   - Proper reference counting

2. **Thread Safety**
   - Uses librmn's native thread safety
   - Minimal Python-level locking
   - Safe file handle management

3. **Error Handling**
   - Direct propagation of librmn errors
   - Clear Python error messages
   - Proper resource cleanup

## Dependencies

- Python ≥3.10
- numpy ≥1.20.0 (for F_CONTIGUOUS support)
- polars ≥0.20.0 (primary DataFrame handling)
- xarray ≥2023.1 (scientific data structures)
- librmn ≥20.0.0 (with FST24 support)

## Usage Examples

### Basic File Reading
```python
from fst24 import Fst24File

with Fst24File("data.fst") as f:
    # Get metadata DataFrame
    df = f.to_polars()
    
    # Get xarray Dataset with forecast axis
    ds = f.to_xarray(forecast_axis=True)
```

### Virtual Zarr Access
```python
from fst24.zarr import FSTZarrArray

# Access FST file as Zarr array
zarr_array = FSTZarrArray("data.fst")

# Convert to Dask array for distributed computing
dask_array = zarr_array.to_dask()

# Direct chunk access
chunk = zarr_array.zarr_array[0:100, 0:100]
```

### Web Map Service
```python
from fst24.ogc import FSTWebMapService

# Create WMS service
wms = FSTWebMapService("data.fst")

# Get map for specific region
bbox = (-100, 30, -60, 60)  # lon1, lat1, lon2, lat2
map_data = wms.get_map(bbox, width=800, height=600, 
                      layers=['TT', 'HU'])
```

### Parallel Access
```python
# Parallel access across runs
files = [f"hrdps/pres/{run}" for run in runs]
with ThreadPoolExecutor() as executor:
    results = executor.map(process_file, files)
```

## Advanced Features

### Virtual Zarr Store
- Direct access to FST files through Zarr interface
- Natural chunking based on FST record structure
- Integration with Dask for distributed computing
- Memory-efficient data access

### OGC API Support
- WMS-compliant interface
- Efficient map generation
- LRU caching for common requests
- Memory-aware rendering

### Performance Optimizations
- Zero-copy array views when possible
- Smart caching strategies
- Efficient coordinate transformations
- Parallel request handling

## Design Considerations

1. **Memory Views**
   - Safe handling of Fortran array views
   - Proper cleanup to prevent memory leaks
   - Copy-on-write for mutable operations

2. **Performance**
   - Direct librmn memory access
   - Minimal data copying
   - Efficient metadata handling

3. **Integration**
   - CF conventions from fstd2nc
   - Polars for metadata handling
   - Support for EMET integration

## Integration Examples

### ECMWF Integration
```python
# Example of ECMWF-style data access
from fst24.ecmwf import ECMWFCompatibleReader

reader = ECMWFCompatibleReader("data.fst")
data = reader.read(
    param=['t', 'q'],
    level=[850, 500],
    time=['00', '12'],
    area=[60, -100, 30, -60]  # N/W/S/E
)
```

### Web API Integration
```python
from fst24.web import FSTWebAPI

# Create web API service
api = FSTWebAPI("data.fst")

# OGC-compliant endpoints
capabilities = api.get_capabilities()
coverage = api.get_coverage(
    variable="TT",
    bbox=(-100, 30, -60, 60),
    time="2024-01-01T00:00:00Z"
)
```

## References

- [fstpy](https://gitlab.science.gc.ca/CMDS/fstpy)
- [fstd2nc](https://github.com/neishm/fstd2nc)
- [librmn](https://gitlab.science.gc.ca/RPN-SI/librmn)
- [EMET](https://gitlab.science.gc.ca/Emet/EMET)
- [spookipy](https://gitlab.science.gc.ca/cmdw-spooki/spookipy)
- [ECMWF Web API](https://www.ecmwf.int/en/forecasts/access-forecasts/ecmwf-web-api)
- [OGC API](https://www.ogc.org/standards/ogcapi-coverages)
- [Zarr Documentation](https://zarr.readthedocs.io/)
