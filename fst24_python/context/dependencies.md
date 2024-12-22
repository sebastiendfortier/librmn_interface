# Project Dependencies

## Core Dependencies

### Runtime Dependencies
- **numpy>=1.24.0**
  - Array handling
  - Memory views
  - F-contiguous arrays

- **polars>=0.20.0**
  - DataFrame operations
  - Memory-efficient string handling
  - Batch processing

- **xarray>=2023.12.0**
  - Dataset handling
  - CF conventions
  - Coordinate systems

- **typing_extensions>=4.8.0**
  - Type hints
  - Protocol support
  - Runtime checking

### System Dependencies
- **librmn>=20.0.0**
  - FST file operations
  - Grid operations
  - Coordinate transformations

## Development Dependencies

### Testing
- **pytest>=7.4.0**
  - Unit testing
  - Fixtures
  - Test discovery

- **pytest-benchmark>=4.0.0**
  - Performance testing
  - Timing measurements
  - Regression checks

- **pytest-cov>=4.1.0**
  - Coverage reporting
  - Branch analysis
  - Missing tests

### Code Quality
- **black>=23.12.0**
  - Code formatting
  - Style consistency
  - PEP 8 compliance

- **mypy>=1.7.0**
  - Static type checking
  - Type inference
  - Protocol validation

- **ruff>=0.1.0**
  - Fast linting
  - Code analysis
  - Style checking

### Documentation
- **sphinx>=7.2.0**
  - API documentation
  - HTML generation
  - Cross-references

- **sphinx-rtd-theme>=2.0.0**
  - Documentation theme
  - Responsive design
  - Search functionality

## Optional Dependencies

### Performance
- **dask>=2023.12.0**
  - Parallel processing
  - Lazy evaluation
  - Memory management

### Development Tools
- **ipython>=8.18.0**
  - Interactive development
  - Debugging
  - Profile analysis

- **ipdb>=0.13.0**
  - Interactive debugging
  - Breakpoints
  - Stack inspection

## Environment Variables

### Required
- **CMCCONST**
  - Path to constants directory
  - Required for metadata decoding
  - Example: `/home/smco502/datafiles/constants`

### Optional
- **FST_OPTIONS**
  - Backend selection
  - Performance tuning
  - Example: `BACKEND=RSF`

## Version Compatibility

### Python Versions
- Minimum: Python 3.10
- Recommended: Python 3.11
- Development: Python 3.12

### Platform Support
- Linux (primary)
- Windows (experimental)
- macOS (not supported)

## Installation

### pip
```bash
# Core installation
pip install fst24-python

# Development installation
pip install -e ".[dev]"

# Full installation with optional dependencies
pip install -e ".[dev,perf,tools]"
```

### conda
```bash
# Core installation
conda install -c conda-forge fst24-python

# Development environment
conda env create -f environment.yml
```

## Dependency Groups

### Core
```toml
[project]
dependencies = [
    "numpy>=1.24.0",
    "polars>=0.20.0",
    "xarray>=2023.12.0",
    "typing_extensions>=4.8.0"
]
```

### Development
```toml
[project.optional-dependencies]
dev = [
    "pytest>=7.4.0",
    "pytest-benchmark>=4.0.0",
    "pytest-cov>=4.1.0",
    "black>=23.12.0",
    "mypy>=1.7.0",
    "ruff>=0.1.0",
    "sphinx>=7.2.0",
    "sphinx-rtd-theme>=2.0.0"
]
```

### Performance
```toml
[project.optional-dependencies]
perf = [
    "dask>=2023.12.0"
]
```

### Development Tools
```toml
[project.optional-dependencies]
tools = [
    "ipython>=8.18.0",
    "ipdb>=0.13.0"
]
```
```


