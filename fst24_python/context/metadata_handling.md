# Metadata Handling Reference

## 1. Core Metadata Structure

### Direct FST Fields
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

## 2. IP Decoding (from fstpy)

### IP1 - Level Encoding
```python
def decode_ip1(ip1: int) -> Dict[str, Any]:
    """
    Decode IP1 value following fstpy conventions
    Reference: fstpy.decode_ip1
    """
    # Kind is stored in top byte
    kind = (ip1 >> 24) & 0xFF
    
    # Value in bottom 24 bits
    value = ip1 & 0x00FFFFFF
    
    if kind == 0:  # Pressure
        return {
            'type': 'pressure',
            'value': value / 100.0,  # Pa to mb
            'unit': 'mb'
        }
    elif kind == 1:  # Height
        return {
            'type': 'height',
            'value': value,
            'unit': 'm'
        }
    elif kind == 2:  # Hybrid
        return {
            'type': 'hybrid',
            'value': value / 10000.0,  # Scaled value
            'unit': 'sigma'
        }
    elif kind == 3:  # User defined
        return {
            'type': 'arbitrary',
            'value': value
        }
```

### IP2 - Time Encoding
```python
def decode_ip2(ip2: int, deet: int = None) -> Dict[str, Any]:
    """
    Decode IP2 value following fstpy conventions
    Reference: fstpy.decode_ip2
    """
    # Kind is stored in top byte
    kind = (ip2 >> 24) & 0xFF
    
    # Value in bottom 24 bits
    value = ip2 & 0x00FFFFFF
    
    if kind == 0:  # Forecast hour
        return {
            'type': 'forecast',
            'value': value,
            'unit': 'hour'
        }
    elif kind == 1 and deet:  # Time steps
        return {
            'type': 'timestep',
            'value': value * deet,  # Convert to seconds
            'unit': 'second'
        }
    elif kind == 2:  # Statistical processing
        return {
            'type': 'stat',
            'value': value,
            'unit': 'hour'
        }
```

### IP3 - User Defined
```python
def decode_ip3(ip3: int) -> Dict[str, Any]:
    """
    Decode IP3 value following fstpy conventions
    Reference: fstpy.decode_ip3
    """
    # Kind is stored in top byte
    kind = (ip3 >> 24) & 0xFF
    
    # Value in bottom 24 bits
    value = ip3 & 0x00FFFFFF
    
    if kind == 1:  # Ensemble member
        return {
            'type': 'ensemble',
            'value': value
        }
    elif kind == 2:  # Statistical ID
        return {
            'type': 'stat_id',
            'value': value
        }
    else:  # User defined
        return {
            'type': 'user',
            'value': value
        }
```

## 3. Extended Metadata

### Variable Information
```python
@dataclass
class VariableInfo:
    """Information from variable dictionary"""
    standard_name: str    # CF standard name
    unit: str            # SI unit
    description: str     # Long description
    bounded: bool        # Has bounds
    accumulation: bool   # Is accumulated
```

### Dictionary Access
```python
class VariableDictionary:
    """Access to CMC variable dictionary"""
    def __init__(self, path: str = None):
        self._path = path or os.environ.get('CMCCONST')
        self._cache = {}
        
    def get_info(self, nomvar: str) -> VariableInfo:
        """Get information for variable"""
        if nomvar not in self._cache:
            self._cache[nomvar] = self._load_info(nomvar)
        return self._cache[nomvar]
```

## 4. Performance Strategy

### Fast Path (Core Metadata)
1. Direct memory access to FST record
2. No IP decoding
3. No dictionary lookups
4. Batch operations where possible

### Extended Path (Optional)
1. Lazy evaluation of IP decoding
2. Cache variable dictionary lookups
3. Reuse decoded values
4. Batch processing support

### Memory Management
1. Zero-copy string handling
2. Efficient cache structure
3. Proper cleanup of resources
4. Thread-safe dictionary access
