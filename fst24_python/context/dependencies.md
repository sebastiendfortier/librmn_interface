# Project Dependencies

## Core Dependencies
- Python>=3.10.0
- numpy>=1.20.0  # For F_CONTIGUOUS array support
- polars>=0.20.0 # Primary DataFrame handling
- dask>=2023.1.0 # For distributed computing and lazy loading
- xarray>=2023.1 # Scientific data structures
- typing_extensions>=4.0.0  # For Python type hints
- dataclasses>=0.8  # For structured data

## Build Dependencies
- librmn>=20.0.0  # Core FST library with FST24 support
- cmake>=3.15.0   # Build system
- gcc>=8.0.0      # C compiler with OpenMP support

## Optional Dependencies
- pandas>=1.3.0  # Optional DataFrame support via Polars conversion
- zarr>=2.14.0   # For chunked array support

## Development Dependencies
- pytest>=7.0.0
- pytest-benchmark>=4.0.0  # Performance testing against FSTPY
- pytest-cov>=4.0.0       # Coverage reporting
- mypy>=1.0.0            # Type checking
- black>=23.0.0          # Code formatting
- ruff>=0.1.0           # Fast Python linter

## Documentation Dependencies
- sphinx>=7.0.0
- sphinx-rtd-theme>=1.0.0
- sphinx-autodoc-typehints>=1.0.0

## Performance Notes
- Requires OpenMP-enabled librmn for parallel operations
- Optimized for HPC environments (80+ CPUs, 250GB+ RAM)
- RSF backend recommended for parallel write operations


