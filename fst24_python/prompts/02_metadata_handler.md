Create a metadata handling system for the FST24 Python interface with these requirements:

1. Efficient Metadata Processing:
   - Fast extraction of record metadata using ctypes
   - Batch processing capabilities for multiple records
   - Memory-efficient caching strategies
   - Support for extended metadata fields:
     ```c
     struct fst_record {
         int dateo;    // Origin timestamp
         int datev;    // Valid timestamp
         int deet;     // Timestep length
         int npas;     // Timestep number
         int ni, nj, nk; // Dimensions
         int ip1, ip2, ip3; // Level, forecast, user-defined
         char typvar[2]; // Field type
         char nomvar[4]; // Variable name
         char etiket[12]; // Label
         char grtyp[1];  // Grid type
         int ig1, ig2, ig3, ig4; // Grid parameters
     }
     ```

2. DataFrame Integration:
   - Primary support for polars DataFrame:
     ```python
     def to_polars(self) -> pl.DataFrame:
         """Convert metadata to polars DataFrame"""
         return pl.DataFrame({
             'nomvar': self.nomvar_list,
             'typvar': self.typvar_list,
             # ... other metadata fields
         })
     ```
   - Optional pandas conversion support
   - Column type optimization
   - Efficient bulk operations

3. Key Metadata Features:
   - IP1/IP2/IP3 decoding using librmn functions
   - Grid type detection and parameters
   - Date/time handling with proper timezone support
   - Extended attribute support

4. Grid Information:
   - Grid type classification
   - Coordinate system parameters
   - Support for:
     - Rotated pole grids
     - Lat/lon grids
     - Polar stereographic
     - U grids
   - Future libgeoref bridge support

5. Performance Requirements:
   - Minimize memory allocations
   - Efficient bulk operations
   - Thread-safe implementations
   - Cache-friendly data structures

Please provide the implementation with detailed comments and documentation.
