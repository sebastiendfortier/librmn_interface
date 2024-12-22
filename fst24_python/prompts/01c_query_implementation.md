# Query Interface Implementation

## Requirements

1. Core Query Class:
```python
from typing import Optional, Iterator, Dict, Any
from dataclasses import dataclass

@dataclass
class QueryCriteria:
    """Search criteria for FST records"""
    nomvar: Optional[str] = None
    typvar: Optional[str] = None
    ip1: Optional[int] = None
    ip2: Optional[int] = None
    ip3: Optional[int] = None
    datev: Optional[int] = None
    etiket: Optional[str] = None

class Fst24Query:
    """FST record query interface with iterator support"""
    
    def __init__(self, file_handle: int, **kwargs):
        """Initialize query with optional search criteria
        
        Args:
            file_handle: FST file handle
            **kwargs: Search criteria (nomvar, typvar, etc.)
        """
        self._handle = file_handle
        self._criteria = QueryCriteria(**kwargs)
        self._current: Optional[Fst24Record] = None
        self._setup_query()
    
    def _setup_query(self) -> None:
        """Configure query parameters in C library"""
        criteria_dict = {k: v for k, v in self._criteria.__dict__.items() 
                        if v is not None}
        self._lib.c_fst24_setup_query(self._handle, 
                                    c_char_p(str(criteria_dict).encode()))
    
    def __iter__(self) -> Iterator[Fst24Record]:
        """Make query iterable"""
        return self
    
    def __next__(self) -> Fst24Record:
        """Get next matching record"""
        record = self._lib.c_fst24_next_record(self._handle)
        if record is None:
            raise StopIteration
        return Fst24Record.from_c_record(record)
    
    def filter(self, **kwargs) -> 'Fst24Query':
        """Create new query with additional filters"""
        new_criteria = {**self._criteria.__dict__, **kwargs}
        return Fst24Query(self._handle, **new_criteria)
```

2. Test Cases:
```python
def test_query_iteration():
    with Fst24File("test.fst") as f:
        query = f.new_query(nomvar="TT")
        records = list(query)
        assert all(r.nomvar == "TT" for r in records)

def test_query_filtering():
    with Fst24File("test.fst") as f:
        base_query = f.new_query(nomvar="TT")
        filtered = base_query.filter(ip1=1000)
        assert all(r.ip1 == 1000 for r in filtered)
```

## Implementation Notes

1. Thread Safety:
   - Query objects are not shared between threads
   - Each query maintains its own state
   - Use thread-local storage for C handles

2. Performance:
   - Lazy evaluation of records
   - Efficient criteria matching
   - Minimal memory allocation

3. Error Handling:
   - Clear error messages for invalid criteria
   - Proper cleanup of C resources
   - Type validation for filter parameters
