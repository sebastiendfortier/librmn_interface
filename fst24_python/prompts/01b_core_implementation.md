# Core Implementation Details

## 1. File Handler Implementation

```python
from typing import Optional, Iterator
from dataclasses import dataclass
from ctypes import CDLL, c_char_p, c_int, c_void_p, Structure

class Fst24File:
    """FST24 file handler with context management and iteration support"""
    
    def __init__(self, filename: str, *, mode: str = "read"):
        """Initialize FST24 file handler
        
        Args:
            filename: Path to FST file
            mode: Access mode ("read" or "write")
        """
        self._lib = CDLL('librmn.so')
        self._filename = filename.encode('utf-8')
        self._mode = mode.encode('utf-8')
        self._handle: Optional[c_int] = None
        self._setup_c_bindings()
        
    def _setup_c_bindings(self) -> None:
        """Configure ctypes function signatures"""
        self._lib.c_fst24_open.argtypes = [c_char_p, c_char_p]
        self._lib.c_fst24_open.restype = c_int
        self._lib.c_fst24_close.argtypes = [c_int]
        self._lib.c_fst24_close.restype = None
        
    def __enter__(self) -> 'Fst24File':
        """Context manager entry"""
        self._handle = self._lib.c_fst24_open(self._filename, self._mode)
        if self._handle.value < 0:
            raise FSTIOError(f"Failed to open {self._filename}")
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        """Context manager exit with cleanup"""
        if self._handle is not None:
            self._lib.c_fst24_close(self._handle)
            self._handle = None
            
    def new_query(self, **kwargs) -> 'Fst24Query':
        """Create new query for record filtering"""
        return Fst24Query(self._handle, **kwargs)
        
    def __iter__(self) -> Iterator[Fst24Record]:
        """Default iterator using unfiltered query"""
        return iter(self.new_query())
```

## 2. Test Requirements

1. Basic Operations:
```python
def test_file_operations():
    with Fst24File("test.fst") as f:
        assert f._handle is not None
    assert f._handle is None  # Verify cleanup
```

2. Error Cases:
```python
def test_file_errors():
    with pytest.raises(FSTIOError):
        with Fst24File("nonexistent.fst"):
            pass
```

## 3. Implementation Notes

1. Thread Safety:
   - Use OpenMP thread-local storage for handles
   - Implement proper locking for shared resources
   - Validate thread safety in tests

2. Error Handling:
   - Clear error messages with context
   - Proper resource cleanup
   - Type validation for inputs

3. Performance:
   - Minimize system calls
   - Efficient handle management
   - Cache-friendly operations
