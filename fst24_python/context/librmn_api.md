# LibRMN C API Reference

## 1. Core FST Functions

```c
// File Operations
int c_fst24_open(char *filename, char *mode);
int c_fst24_close(int handle);
int c_fst24_find(int handle, void *where);

// Data Access
int c_fst24_getk(int handle, void *buffer, int *ni, int *nj, int *nk);
int c_fst24_putk(int handle, void *buffer, int ni, int nj, int nk);

// Metadata Operations
int c_fst24_get_meta(int handle, fst_record_struct *meta);
int c_fst24_put_meta(int handle, fst_record_struct *meta);

// Grid Operations
int c_fst24_get_gridinfo(int handle, grid_desc_struct *grid);
int c_fst24_get_coords(int handle, float *lat, float *lon);
```

## 2. Data Structures

```c
// Record Metadata
typedef struct {
    int dateo;      // Origin timestamp (CMC date stamp)
    int datev;      // Valid timestamp
    int deet;       // Time step length
    int npas;       // Time step number
    int ni, nj, nk; // Grid dimensions
    int ip1;        // Level (encoded)
    int ip2;        // Time (encoded)
    int ip3;        // User defined
    char typvar[2]; // Type of field
    char nomvar[4]; // Variable name
    char etiket[12];// Label
    char grtyp[1];  // Grid type
    int ig1;        // Grid parameter 1
    int ig2;        // Grid parameter 2
    int ig3;        // Grid parameter 3
    int ig4;        // Grid parameter 4
    int datyp;      // Data type code
    int nbits;      // Number of bits per value
} fst_record_struct;

// Grid Description
typedef struct {
    char grtyp[1];  // Grid type
    int ig1;        // First parameter
    int ig2;        // Second parameter
    int ig3;        // Third parameter
    int ig4;        // Fourth parameter
    float *ax;      // X-axis coordinates
    float *ay;      // Y-axis coordinates
} grid_desc_struct;
```

## 3. Error Codes

```c
#define FST_OK           0    // Success
#define FST_ERROR      -1    // General error
#define FST_NOT_FOUND  -2    // Record not found
#define FST_EOF       -3    // End of file
#define FST_BAD_HANDLE -4    // Invalid file handle
#define FST_BAD_ARGS  -5    // Invalid arguments
```

## 4. Data Types

```c
// FST Data Types
#define FST_TYPE_FLOAT    1   // 32-bit float
#define FST_TYPE_DOUBLE   2   // 64-bit float
#define FST_TYPE_INT      3   // 32-bit integer
#define FST_TYPE_CHAR     4   // Character string
#define FST_TYPE_PACKED   5   // Packed data
#define FST_TYPE_COMP     6   // Compressed data
```

## 5. Environment Variables

```bash
# Backend Selection
FST_OPTIONS="BACKEND=RSF"    # Use RSF backend
FST_OPTIONS="BACKEND=XDF"    # Use XDF backend (default)

# Performance Tuning
FST_CACHE_SIZE=256          # Cache size in MB
FST_MAX_THREADS=4           # Maximum number of threads
```

## 6. Thread Safety

All functions are thread-safe when using the RSF backend. The XDF backend has limited thread safety and should be used with appropriate locking mechanisms.

## 7. Error Handling

Functions return negative values on error. Use `c_fst24_error_message(int code)` to get human-readable error messages.

## 8. Memory Management

- All memory allocated by the library must be freed using `c_fst24_free()`
- Use `c_fst24_clean()` for cleanup of thread-local resources
- Memory mapping is handled automatically based on file size
