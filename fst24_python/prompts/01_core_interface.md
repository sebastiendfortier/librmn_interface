Create a modern Python interface for the FST24 file format in librmn with these requirements:

1. Use the new FST24/RSF backend capabilities:
   - Support for both XDF and RSF formats through environment variable FST_OPTIONS="BACKEND=RSF"
   - Thread-safe operations using OpenMP/threading
   - Parallel write support for RSF format
   - Extended metadata handling via fst24 API

2. Follow these Python best practices:
   - Use ctypes for C bindings to avoid Python version dependencies
   - Implement Pythonic iteration patterns:
     ```python
     with Fst24File("file.fst") as f:
         for record in f:  # Uses new_query() internally
             process(record)
     ```
   - Support context managers for file handling
   - Use type hints and dataclasses where appropriate

3. Core classes needed:
   - Fst24File:
     ```python
     class Fst24File:
         def __init__(self, filename: str, *, mode: str = "read")
         def new_query(self, **kwargs) -> Fst24Query
         def __iter__(self) -> Iterator[Fst24Record]
     ```
   - Fst24Query: Search and iteration interface
   - Fst24Record: Record data and metadata container

4. Key features to implement:
   - Efficient metadata extraction using ctypes
   - Support for polars/pandas DataFrame conversion
   - xarray integration capabilities
   - Bridge interface for future libgeoref integration
   - Support for F_CONTIGUOUS numpy arrays

5. Error Handling:
   - Proper resource cleanup
   - Clear error messages
   - Type checking for inputs
   - Thread safety validation

Please provide the implementation with detailed comments and documentation.
