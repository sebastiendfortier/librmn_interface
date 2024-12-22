Create a data access layer for the FST24 Python interface with these requirements:

1. Efficient Data Access:
   - Direct memory access using numpy arrays
   - Support for F_CONTIGUOUS arrays
   - Proper handling of data types:
     ```python
     def get_data(self) -> np.ndarray:
         """Get record data as numpy array"""
         dtype = self._get_numpy_dtype()
         shape = (self.ni, self.nj, self.nk)
         return np.array(self._data, dtype=dtype).reshape(shape, order='F')
     ```
   - Memory mapping for large files

2. xarray Integration:
   - Automatic coordinate detection
   - Grid information handling:
     ```python
     def to_xarray(self) -> xr.DataArray:
         """Convert to xarray with proper coordinates"""
         data = self.get_data()
         coords = {
             'lat': self.get_latitudes(),
             'lon': self.get_longitudes(),
             'level': self.get_levels(),
             'time': self.get_valid_dates()
         }
         return xr.DataArray(data, coords=coords)
     ```
   - Support for multi-dimensional data
   - Metadata preservation

3. Data Type Support:
   - Float32/64
   - Integer types
   - Character data
   - Packed data handling
   - Compression support

4. Performance Features:
   - Lazy loading
   - Parallel read support
   - Memory efficient operations
   - Cache optimization

5. Grid Operations:
   - Coordinate calculations
   - Grid transformations
   - Future libgeoref integration:
     ```python
     def get_grid_info(self) -> Dict:
         """Get grid information for georef"""
         return {
             'type': self.grtyp,
             'params': [self.ig1, self.ig2, self.ig3, self.ig4],
             'dimensions': (self.ni, self.nj)
         }
     ```

Please provide the implementation with detailed comments and documentation.
