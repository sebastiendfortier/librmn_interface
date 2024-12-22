# Polars Integration Implementation

## 1. Requirements

### Core Integration
- Efficient metadata extraction
- Support for extended metadata
- Batch record processing
- Memory-efficient operations

### Performance
- Target conversion: < 0.05s
- Minimal memory overhead
- Efficient string handling
- Batch processing optimization

## 2. Implementation Steps

1. **Metadata Schema Definition**
   ```python
   class MetadataSchema:
       """Schema definition for Polars DataFrame"""
       def __init__(self, extended: bool = False):
           self._extended = extended
           
       @property
       def core_schema(self) -> Dict[str, pl.DataType]:
           """Core metadata schema"""
           return {
               'nomvar': pl.Utf8,
               'typvar': pl.Utf8,
               'etiket': pl.Utf8,
               'ni': pl.Int32,
               'nj': pl.Int32,
               'nk': pl.Int32,
               'dateo': pl.Int64,
               'ip1': pl.Int32,
               'ip2': pl.Int32,
               'ip3': pl.Int32,
               'deet': pl.Int32,
               'npas': pl.Int32,
               'datyp': pl.Int32,
               'nbits': pl.Int32,
               'grtyp': pl.Utf8
           }
           
       @property
       def extended_schema(self) -> Dict[str, pl.DataType]:
           """Extended metadata schema"""
           if not self._extended:
               return {}
               
           return {
               'bounded': pl.Boolean,
               'missing_data': pl.Boolean,
               'compressed': pl.Boolean,
               'level_type': pl.Utf8,
               'ip_info': pl.Struct([
                   pl.Field('kind', pl.Int32),
                   pl.Field('level', pl.Float32)
               ])
           }
           
       def get_schema(self) -> Dict[str, pl.DataType]:
           """Get complete schema"""
           schema = self.core_schema.copy()
           schema.update(self.extended_schema)
           return schema
   ```

2. **Record Batch Processing**
   ```python
   class RecordBatchProcessor:
       """Efficient batch processing of records"""
       def __init__(self, records: List[FSTRecord],
                    extended: bool = False):
           self._records = records
           self._schema = MetadataSchema(extended)
           
       def to_polars(self) -> pl.DataFrame:
           """Convert records to Polars DataFrame"""
           # Pre-allocate columns
           columns = {
               name: self._allocate_column(dtype, len(self._records))
               for name, dtype in self._schema.get_schema().items()
           }
           
           # Fill columns in batch
           for i, record in enumerate(self._records):
               self._process_record(record, columns, i)
               
           # Create DataFrame
           return pl.DataFrame(columns)
           
       def _allocate_column(self, dtype: pl.DataType,
                           length: int) -> np.ndarray:
           """Allocate column with appropriate type"""
           if dtype == pl.Utf8:
               return np.empty(length, dtype=object)
           elif dtype == pl.Boolean:
               return np.zeros(length, dtype=bool)
           elif dtype == pl.Int32:
               return np.zeros(length, dtype=np.int32)
           elif dtype == pl.Int64:
               return np.zeros(length, dtype=np.int64)
           elif dtype == pl.Float32:
               return np.zeros(length, dtype=np.float32)
           elif isinstance(dtype, pl.Struct):
               return np.zeros(length, dtype=object)
               
       def _process_record(self, record: FSTRecord,
                          columns: Dict[str, np.ndarray],
                          index: int):
           """Process single record into columns"""
           # Core metadata
           columns['nomvar'][index] = record.nomvar
           columns['typvar'][index] = record.typvar
           columns['etiket'][index] = record.etiket
           columns['ni'][index] = record.ni
           columns['nj'][index] = record.nj
           columns['nk'][index] = record.nk
           columns['dateo'][index] = record.dateo
           columns['ip1'][index] = record.ip1
           columns['ip2'][index] = record.ip2
           columns['ip3'][index] = record.ip3
           columns['deet'][index] = record.deet
           columns['npas'][index] = record.npas
           columns['datyp'][index] = record.datyp
           columns['nbits'][index] = record.nbits
           columns['grtyp'][index] = record.grtyp
           
           # Extended metadata if enabled
           if self._schema._extended:
               self._process_extended(record, columns, index)
               
       def _process_extended(self, record: FSTRecord,
                            columns: Dict[str, np.ndarray],
                            index: int):
           """Process extended metadata"""
           columns['bounded'][index] = record.bounded
           columns['missing_data'][index] = record.missing_data
           columns['compressed'][index] = record.compressed
           columns['level_type'][index] = record.level_type
           columns['ip_info'][index] = {
               'kind': record.ip_kind,
               'level': record.ip_level
           }
   ```

3. **DataFrame Interface**
   ```python
   class PolarsFST:
       """High-level Polars interface"""
       def __init__(self, path: str):
           self._path = path
           self._file = None
           
       def to_polars(self, extended: bool = False) -> pl.DataFrame:
           """Convert FST file to Polars DataFrame"""
           if self._file is None:
               self._file = Fst24File(self._path)
               
           records = self._file.read_records()
           processor = RecordBatchProcessor(records, extended)
           return processor.to_polars()
           
       def __enter__(self):
           """Context manager entry"""
           if self._file is None:
               self._file = Fst24File(self._path)
           return self
           
       def __exit__(self, exc_type, exc_val, exc_tb):
           """Context manager exit"""
           if self._file is not None:
               self._file.close()
               self._file = None
   ```

## 3. Testing Requirements

1. **Schema Handling**
   ```python
   def test_schema_definition():
       schema = MetadataSchema(extended=False)
       core = schema.get_schema()
       assert 'nomvar' in core
       assert 'ip_info' not in core
       
       schema = MetadataSchema(extended=True)
       full = schema.get_schema()
       assert 'ip_info' in full
       assert isinstance(full['ip_info'], pl.Struct)
   ```

2. **Batch Processing**
   ```python
   def test_batch_processing():
       records = create_test_records(100)
       processor = RecordBatchProcessor(records)
       df = processor.to_polars()
       
       assert len(df) == 100
       assert all(col in df.columns
                 for col in processor._schema.core_schema)
   ```

3. **DataFrame Interface**
   ```python
   def test_polars_interface():
       with PolarsFST("test.fst") as fst:
           df = fst.to_polars()
           assert len(df) > 0
           assert all(col in df.columns
                     for col in MetadataSchema().core_schema)
           
           df_ext = fst.to_polars(extended=True)
           assert 'ip_info' in df_ext.columns
   ```

## 4. Performance Requirements

1. **Schema Handling**
   - Schema creation: < 0.1ms
   - Type mapping: < 0.01ms
   - Memory usage: O(1)

2. **Batch Processing**
   - Column allocation: < 1ms
   - Record processing: < 0.1ms per record
   - Memory usage: O(n) where n is record count

3. **DataFrame Creation**
   - Total time: < 0.05s for typical file
   - Memory overhead: < 1.5x metadata size
   - String interning when possible

## 5. Error Handling

1. **Schema Definition**
   - Invalid data types
   - Missing required fields
   - Type conversion errors

2. **Batch Processing**
   - Memory allocation failures
   - Invalid record data
   - Type conversion errors

3. **DataFrame Creation**
   - File access errors
   - Memory constraints
   - Invalid metadata 
