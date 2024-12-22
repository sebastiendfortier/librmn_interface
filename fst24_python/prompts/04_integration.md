# XArray Integration Implementation

## 1. Requirements

### Core Integration
- Match fstd2nc's CF conventions
- Efficient coordinate handling
- Forecast axis support
- Metadata preservation

### Performance
- Target conversion: < 3.5s
- Minimal memory overhead
- Lazy operations where possible
- Efficient coordinate caching

## 2. Implementation Steps

1. **Coordinate System Management**
   ```python
   class CoordinateSystem:
       """Grid coordinate system handler"""
       def __init__(self, record: FSTRecord):
           self._record = record
           self._coords_cache = {}
           
       def get_coordinates(self) -> Dict[str, np.ndarray]:
           """Get coordinate arrays for grid"""
           cache_key = self._make_cache_key()
           if cache_key not in self._coords_cache:
               if self._record.grtyp in ['A', 'B', 'E', 'L']:
                   self._coords_cache[cache_key] = (
                       self._get_geographic_coords()
                   )
               elif self._record.grtyp in ['G', 'N', 'S']:
                   self._coords_cache[cache_key] = (
                       self._get_projected_coords()
                   )
               else:
                   self._coords_cache[cache_key] = (
                       self._get_custom_coords()
                   )
           return self._coords_cache[cache_key]
           
       def _get_geographic_coords(self) -> Dict[str, np.ndarray]:
           """Get lat/lon coordinates"""
           lat = np.empty((self._record.ni, self._record.nj),
                         dtype=np.float32, order='F')
           lon = np.empty((self._record.ni, self._record.nj),
                         dtype=np.float32, order='F')
           
           # Call librmn to get coordinates
           status = self._lib.c_fst24_get_coords(
               self._handle,
               lat.ctypes.data_as(POINTER(c_float)),
               lon.ctypes.data_as(POINTER(c_float))
           )
           
           if status < 0:
               raise FSTError("Failed to get coordinates")
               
           return {
               'latitude': lat,
               'longitude': lon
           }
   ```

2. **CF Convention Handling**
   ```python
   class CFConverter:
       """CF convention handling following fstd2nc"""
       def __init__(self, var_dict: VariableDictionary):
           self._var_dict = var_dict
           
       def get_attributes(self, record: FSTRecord) -> Dict[str, Any]:
           """Get CF-compliant attributes"""
           var_info = self._var_dict.get_info(record.nomvar)
           attrs = {
               'standard_name': var_info.standard_name,
               'units': var_info.unit,
               'long_name': var_info.description
           }
           
           # Add grid mapping if needed
           if record.grtyp in ['N', 'S', 'G']:
               attrs['grid_mapping'] = self._get_grid_mapping(record)
               
           return attrs
           
       def get_dimensions(self, record: FSTRecord,
                         forecast_axis: bool = False) -> List[str]:
           """Get dimension names following CF"""
           dims = []
           
           # Vertical dimension
           if record.nk > 1:
               dims.append(self._get_vertical_dim(record))
               
           # Horizontal dimensions
           if record.grtyp in ['A', 'B', 'E', 'L']:
               dims.extend(['latitude', 'longitude'])
           else:
               dims.extend(['y', 'x'])
               
           # Time dimension
           if forecast_axis:
               dims.append('forecast')
               
           return dims
   ```

3. **XArray Dataset Creation**
   ```python
   class XArrayConverter:
       """Efficient xarray conversion"""
       def __init__(self, records: List[Fst24Record],
                    forecast_axis: bool = False):
           self._records = records
           self._forecast_axis = forecast_axis
           self._cf = CFConverter(VariableDictionary())
           self._coords = CoordinateSystem(records[0])
           
       def to_dataset(self) -> xr.Dataset:
           """Convert records to xarray Dataset"""
           ds = xr.Dataset()
           
           # Add coordinates first
           coords = self._coords.get_coordinates()
           for name, values in coords.items():
               ds.coords[name] = values
               
           # Add forecast coordinate if needed
           if self._forecast_axis:
               ds.coords['forecast'] = self._get_forecast_coord()
               
           # Add variables
           for record in self._records:
               var_name = record.nomvar.strip()
               dims = self._cf.get_dimensions(record,
                                            self._forecast_axis)
               attrs = self._cf.get_attributes(record)
               
               ds[var_name] = xr.DataArray(
                   data=record.data,
                   dims=dims,
                   coords={k: ds.coords[k] for k in dims},
                   attrs=attrs
               )
               
           return ds
           
       def _get_forecast_coord(self) -> np.ndarray:
           """Get forecast hours"""
           unique_ip2s = sorted(set(
               r.ip2 for r in self._records
           ))
           return np.array(unique_ip2s, dtype=np.int32)
   ```

## 3. Testing Requirements

1. **Coordinate Systems**
   ```python
   def test_coordinate_systems():
       # Geographic grid
       record = create_test_record(grtyp='A')
       coords = CoordinateSystem(record).get_coordinates()
       assert 'latitude' in coords
       assert 'longitude' in coords
       
       # Projected grid
       record = create_test_record(grtyp='N')
       coords = CoordinateSystem(record).get_coordinates()
       assert 'x' in coords
       assert 'y' in coords
   ```

2. **CF Conventions**
   ```python
   def test_cf_compliance():
       converter = CFConverter(var_dict)
       attrs = converter.get_attributes(record)
       assert 'standard_name' in attrs
       assert 'units' in attrs
       
       dims = converter.get_dimensions(record, forecast_axis=True)
       assert all(d in ['latitude', 'longitude', 'forecast']
                 for d in dims)
   ```

3. **Dataset Creation**
   ```python
   def test_dataset_creation():
       converter = XArrayConverter(records, forecast_axis=True)
       ds = converter.to_dataset()
       
       # Check coordinates
       assert all(c in ds.coords
                 for c in ['latitude', 'longitude', 'forecast'])
       
       # Check variables
       for record in records:
           var = record.nomvar.strip()
           assert var in ds
           assert ds[var].attrs['units'] is not None
   ```

## 4. Performance Requirements

1. **Coordinate Handling**
   - Cache hit rate: > 95%
   - Coordinate generation: < 100ms
   - Memory usage: O(ni * nj) per grid type

2. **Dataset Creation**
   - Total time: < 3.5s for typical file
   - Memory overhead: < 2x data size
   - Lazy operations where possible

3. **CF Conversion**
   - Attribute lookup: < 1ms
   - Dimension inference: < 0.1ms
   - Grid mapping: < 10ms

## 5. Error Handling

1. **Coordinate Generation**
   - Invalid grid types
   - Coordinate calculation errors
   - Memory allocation failures

2. **CF Compliance**
   - Missing standard names
   - Unknown units
   - Invalid grid mappings

3. **Dataset Creation**
   - Dimension mismatch
   - Coordinate inconsistency
   - Memory constraints
