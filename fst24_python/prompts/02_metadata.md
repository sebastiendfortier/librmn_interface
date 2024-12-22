# Metadata Handling Implementation

## 1. Requirements

### Core Metadata Handling
- Zero-copy extraction from FST records
- Fast path without decoding
- Batch operations support
- Thread-safe access

### Extended Metadata (Optional)
- IP1/2/3 decoding from fstpy
- Variable dictionary integration
- Lazy evaluation
- Cache management

## 2. Implementation Steps

1. **Core Metadata Structure**
   ```python
   @dataclass
   class CoreMetadata:
       """Fast-path metadata extraction"""
       nomvar: str    # Variable name (4 chars)
       typvar: str    # Type of field (2 chars)
       etiket: str    # Label (12 chars)
       datev: int     # Valid date stamp
       ip1: int       # Level encoding
       ip2: int       # Time encoding
       ip3: int       # User defined
       ni: int        # First dimension
       nj: int        # Second dimension
       nk: int        # Third dimension
       grtyp: str     # Grid type (1 char)
       ig1: int       # First grid descriptor
       ig2: int       # Second grid descriptor
       ig3: int       # Third grid descriptor
       ig4: int       # Fourth grid descriptor
   ```

2. **IP Decoding**
   ```python
   class IPDecoder:
       """IP1/2/3 decoding following fstpy conventions"""
       
       def decode_ip1(self, ip1: int) -> Dict[str, Any]:
           """Decode level information"""
           kind = (ip1 >> 24) & 0xFF
           value = ip1 & 0x00FFFFFF
           
           if kind == 0:  # Pressure
               return {
                   'type': 'pressure',
                   'value': value / 100.0,  # Pa to mb
                   'unit': 'mb'
               }
           # ... other decodings
           
       def decode_ip2(self, ip2: int, deet: int = None) -> Dict[str, Any]:
           """Decode time information"""
           kind = (ip2 >> 24) & 0xFF
           value = ip2 & 0x00FFFFFF
           # ... implementation
           
       def decode_ip3(self, ip3: int) -> Dict[str, Any]:
           """Decode user information"""
           kind = (ip3 >> 24) & 0xFF
           value = ip3 & 0x00FFFFFF
           # ... implementation
   ```

3. **Variable Dictionary**
   ```python
   class VariableDictionary:
       """Thread-safe variable dictionary access"""
       def __init__(self, path: str = None):
           self._path = path or os.environ.get('CMCCONST')
           self._cache = {}
           self._lock = threading.Lock()
           
       def get_info(self, nomvar: str) -> VariableInfo:
           """Get cached variable information"""
           with self._lock:
               if nomvar not in self._cache:
                   self._cache[nomvar] = self._load_info(nomvar)
               return self._cache[nomvar]
   ```

4. **Polars Integration**
   ```python
   class MetadataHandler:
       """Efficient metadata handling"""
       def __init__(self, records: List[Fst24Record],
                    decode_metadata: bool = False):
           self._records = records
           self._decode = decode_metadata
           self._ip_decoder = IPDecoder() if decode_metadata else None
           self._var_dict = (VariableDictionary() 
                            if decode_metadata else None)
           
       def to_polars(self) -> pl.LazyFrame:
           """Convert to polars with optional decoding"""
           # Fast path: core metadata only
           df = pl.LazyFrame({
               'nomvar': [r.nomvar for r in self._records],
               'typvar': [r.typvar for r in self._records],
               'etiket': [r.etiket for r in self._records],
               'datev': [r.datev for r in self._records],
               'ip1': [r.ip1 for r in self._records],
               'ip2': [r.ip2 for r in self._records],
               'ip3': [r.ip3 for r in self._records]
           })
           
           # Optional: extended metadata
           if self._decode:
               df = df.with_columns([
                   pl.col('ip1')
                     .map_elements(self._ip_decoder.decode_ip1)
                     .alias('level'),
                   pl.col('ip2')
                     .map_elements(self._ip_decoder.decode_ip2)
                     .alias('time'),
                   pl.col('ip3')
                     .map_elements(self._ip_decoder.decode_ip3)
                     .alias('extra'),
                   pl.col('nomvar')
                     .map_elements(self._var_dict.get_info)
                     .alias('var_info')
               ])
           
           return df
   ```

## 3. Testing Requirements

1. **Core Metadata**
   ```python
   def test_core_metadata():
       handler = MetadataHandler(records)
       df = handler.to_polars()
       assert len(df.columns) == 7  # Core fields only
       assert df.shape[0] == len(records)
   ```

2. **Extended Metadata**
   ```python
   def test_extended_metadata():
       handler = MetadataHandler(records, decode_metadata=True)
       df = handler.to_polars()
       assert 'level' in df.columns
       assert 'time' in df.columns
       assert 'var_info' in df.columns
   ```

3. **Thread Safety**
   ```python
   def test_thread_safety():
       handler = MetadataHandler(records, decode_metadata=True)
       with ThreadPoolExecutor() as ex:
           results = list(ex.map(
               lambda x: handler.to_polars(),
               range(10)
           ))
   ```

## 4. Performance Requirements

1. **Core Metadata (Fast Path)**
   - Extraction: < 0.03s for 179 records
   - Memory overhead: Minimal
   - No allocations for strings

2. **Extended Metadata**
   - Decoding: < 0.12s for 179 records
   - Cache hit rate: > 90%
   - Thread-safe dictionary access

3. **Memory Management**
   - Zero-copy string handling
   - Efficient cache structure
   - Clean resource handling

## 5. Error Handling

1. **Dictionary Access**
   - Missing variables
   - Invalid dictionary path
   - XML parsing errors

2. **IP Decoding**
   - Unknown kind values
   - Invalid encodings
   - Missing deet values

3. **Resource Cleanup**
   - Dictionary cache limits
   - Memory cleanup
   - Thread safety
