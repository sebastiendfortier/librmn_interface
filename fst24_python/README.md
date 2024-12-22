# FST24 Python Interface

A modern Python interface for the FST24 file format, providing efficient access to meteorological data with support for both XDF and RSF backends.

## Features

- Fast metadata extraction and data access
- Support for both XDF and RSF backends
- Memory-efficient operations with memory mapping
- Thread-safe file operations
- Polars and xarray integration
- Batch processing capabilities
- Performance optimized for large files

## Installation

```bash
pip install fst24-python
```

## Quick Start

```python
import fst24

# Read metadata as Polars DataFrame
with fst24.open("path/to/file.fst") as f:
    df = f.to_polars()

# Read data as xarray Dataset
with fst24.open("path/to/file.fst") as f:
    ds = f.to_xarray(forecast_axis=True)

# Memory-efficient batch processing
with fst24.open("path/to/file.fst") as f:
    for record in f.iter_records():
        data = record.data  # Lazy loading
```

## Performance

Benchmarks against baseline implementations:

1. **Metadata Reading**
   ```python
   # FST24 (0.03s)
   df = fst24.read_metadata("file.fst")
   
   # FSTPY (0.10s)
   df = fstpy.StandardFileReader("file.fst").to_pandas()
   ```

2. **XArray Conversion**
   ```python
   # FST24 (3.5s)
   ds = fst24.to_xarray("file.fst", forecast_axis=True)
   
   # FSTD2NC (3.5s)
   ds = fstd2nc.Buffer("file.fst", forecast_axis=True).to_xarray()
   ```

## Project Structure

```
fst24_python/
├── context/                 # Context and design documents
│   ├── design_decisions.md
│   ├── dependencies.md
│   ├── librmn_api.md
│   └── metadata_handling.md
├── prompts/                 # Implementation prompts
│   ├── 00_implementation_order.md
│   ├── 01_core.md
│   ├── 02_metadata.md
│   ├── 03_data_access.md
│   ├── 04_integration.md
│   └── 05_polars.md
├── src/                     # Source code
│   └── fst24/
│       ├── __init__.py
│       ├── core.py
│       ├── metadata.py
│       ├── data.py
│       ├── polars.py
│       └── xarray.py
├── tests/                   # Test suite
│   ├── test_core.py
│   ├── test_metadata.py
│   ├── test_data.py
│   ├── test_polars.py
│   └── test_xarray.py
├── README.md
├── pyproject.toml
└── setup.py
```

## Implementation Plan

### Phase 1: Core Interface (1-2 days)
- Basic file operations
- Record reading and writing
- Metadata handling
- Thread safety

### Phase 2: Data Access (2-3 days)
- Memory mapping
- Array views
- Performance optimization
- Resource management

### Phase 3: Integration (2-3 days)
- Polars integration
- XArray integration
- CF conventions
- Performance tuning

### Phase 4: Testing (2-3 days)
- Unit tests
- Integration tests
- Benchmarks
- Documentation

## Development Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/your-org/fst24-python.git
   cd fst24-python
   ```

2. **Create virtual environment**
   ```bash
   python -m venv venv
   source venv/bin/activate  # or venv\Scripts\activate on Windows
   ```

3. **Install dependencies**
   ```bash
   pip install -e ".[dev]"
   ```

4. **Run tests**
   ```bash
   pytest tests/
   ```

## Configuration

### Backend Selection
```bash
export FST_OPTIONS="BACKEND=RSF"  # or XDF
```

### Memory Management
```python
import fst24
fst24.set_memory_limit(16 * 1024**3)  # 16GB limit
```

### Thread Safety
```python
import fst24
fst24.enable_thread_safety()
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.
