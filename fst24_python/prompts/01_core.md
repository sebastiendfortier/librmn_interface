# Core File Interface Implementation

## 1. Requirements

### Direct librmn Integration
- Use ctypes for C bindings
- Support both RSF and XDF backends
- Thread-safe operations
- Proper error handling

### Core Classes
```python
class Fst24File:
    """Main file interface"""
    def __init__(self, path: str, mode: str = 'r'):
        """Initialize file handle"""
        pass
        
    def __enter__(self) -> 'Fst24File':
        """Context manager entry"""
        pass
        
    def __exit__(self, *args):
        """Context manager exit with cleanup"""
        pass
        
    def read_records(self) -> List[Fst24Record]:
        """Read all records"""
        pass

class Fst24Record:
    """Record container"""
    def __init__(self, handle: int, record: FSTRecord):
        """Initialize from C structure"""
        pass
        
    @property
    def data(self) -> np.ndarray:
        """Lazy load data"""
        pass

class Fst24Query:
    """Search interface"""
    def __init__(self, handle: int, **criteria):
        """Initialize search"""
        pass
        
    def __iter__(self) -> Iterator[Fst24Record]:
        """Iterate over matching records"""
        pass
```

## 2. Implementation Steps

1. **C Bindings Setup**
   ```python
   # Record structure mapping
   class FSTRecord(Structure):
       _fields_ = [
           ('dateo', c_int32),
           ('datev', c_int32),
           ('deet', c_int32),
           ('npas', c_int32),
           ('ni', c_int32),
           ('nj', c_int32),
           ('nk', c_int32),
           ('ip1', c_int32),
           ('ip2', c_int32),
           ('ip3', c_int32),
           ('typvar', c_char * 2),
           ('nomvar', c_char * 4),
           ('etiket', c_char * 12),
           ('grtyp', c_char * 1),
           ('ig1', c_int32),
           ('ig2', c_int32),
           ('ig3', c_int32),
           ('ig4', c_int32),
           ('datyp', c_int32),
           ('nbits', c_int32)
       ]
   
   # Function bindings
   lib = CDLL('librmn.so')
   lib.c_fst24_open.argtypes = [c_char_p, c_char_p]
   lib.c_fst24_open.restype = c_int32
   ```

2. **Backend Selection**
   ```python
   def _get_backend() -> str:
       """Get backend from environment"""
       options = os.environ.get('FST_OPTIONS', '')
       if 'BACKEND=RSF' in options:
           return 'RSF'
       return 'XDF'
   ```

3. **Thread Safety**
   ```python
   class FileHandle:
       """Thread-safe file handle"""
       def __init__(self, path: str, mode: str):
           self._lock = threading.Lock() if _get_backend() == 'XDF' else None
           
       def __enter__(self):
           if self._lock:
               self._lock.acquire()
           
       def __exit__(self, *args):
           if self._lock:
               self._lock.release()
   ```

4. **Error Handling**
   ```python
   def check_error(code: int) -> None:
       """Check return code"""
       if code < 0:
           msg = lib.c_fst24_error_message(code)
           raise FSTError(msg.decode('utf-8'))
   ```

## 3. Testing Requirements

1. **File Operations**
   ```python
   def test_file_operations():
       with Fst24File("test.fst") as f:
           assert f._handle > 0
       # Handle should be closed
   ```

2. **Thread Safety**
   ```python
   def test_thread_safety():
       file = Fst24File("test.fst")
       with ThreadPoolExecutor() as ex:
           results = ex.map(lambda x: file.read_records(), range(10))
   ```

3. **Error Cases**
   ```python
   def test_error_handling():
       with pytest.raises(FSTError):
           Fst24File("nonexistent.fst")
   ```

## 4. Performance Requirements

1. **File Operations**
   - Open/close: < 1ms
   - Record iteration: Linear scaling
   - Memory cleanup: Immediate

2. **Thread Safety**
   - RSF: No overhead
   - XDF: Minimal locking impact
   - Handle limit: 998 files

3. **Error Handling**
   - Fast error checks
   - Clear messages
   - No memory leaks
