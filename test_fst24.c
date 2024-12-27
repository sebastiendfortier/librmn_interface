#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rmn.h>

int main() {
    printf("Testing FST file reading...\n");
    
    // Open the file
    int iun = 0;
    int status = c_fnom(&iun, "2024122200_002", "RND", 0);
    if (status < 0) {
        printf("Failed to get file handle\n");
        return 1;
    }
    printf("Got file handle: %d\n", iun);
    
    // Open FST file
    status = c_fstouv(iun, "RND");
    if (status < 0) {
        printf("Failed to open FST file\n");
        c_fclos(iun);
        return 1;
    }
    printf("Opened FST file\n");
    
    // Read records
    int ni, nj, nk;
    int key;
    
    // Get first record
    key = c_fstinf(iun, &ni, &nj, &nk, -1, "", -1, -1, -1, "", "");
    if (key < 0) {
        printf("No records found\n");
        c_fstfrm(iun);
        c_fclos(iun);
        return 1;
    }
    
    // Print first record info
    int dateo, deet, npas, nbits;
    int ip1, ip2, ip3, datyp;
    char typvar[3], nomvar[5], etiket[13], grtyp[2];
    int ig1, ig2, ig3, ig4;
    int swa, lng, dltf, ubc;
    int extra1, extra2, extra3;
    
    status = c_fstprm(key, &dateo, &deet, &npas, &ni, &nj, &nk,
                      &nbits, &datyp, &ip1, &ip2, &ip3, typvar,
                      nomvar, etiket, grtyp, &ig1, &ig2, &ig3, &ig4,
                      &swa, &lng, &dltf, &ubc, &extra1, &extra2, &extra3);
                      
    if (status < 0) {
        printf("Failed to get record parameters\n");
        c_fstfrm(iun);
        c_fclos(iun);
        return 1;
    }
    
    printf("\nFirst record info:\n");
    printf("  nomvar: %.4s\n", nomvar);
    printf("  typvar: %.2s\n", typvar);
    printf("  etiket: %.12s\n", etiket);
    printf("  dimensions: %d x %d x %d\n", ni, nj, nk);
    printf("  ip1/2/3: %d, %d, %d\n", ip1, ip2, ip3);
    printf("  grid type: %.1s\n", grtyp);
    printf("  data type: %d\n", datyp);
    printf("  nbits: %d\n", nbits);
    
    // Allocate memory and read data
    float* data = malloc(ni * nj * nk * sizeof(float));
    if (!data) {
        printf("Failed to allocate memory for data\n");
        c_fstfrm(iun);
        c_fclos(iun);
        return 1;
    }
    
    status = c_fstluk(data, key, &ni, &nj, &nk);
    if (status < 0) {
        printf("Failed to read data\n");
        free(data);
        c_fstfrm(iun);
        c_fclos(iun);
        return 1;
    }
    
    // Print some sample values
    printf("\nSample values:\n");
    for (int i = 0; i < ni && i < 3; i++) {
        for (int j = 0; j < nj && j < 3; j++) {
            printf("  data[%d][%d] = %g\n", i, j, data[i * nj + j]);
        }
    }
    
    // Clean up
    free(data);
    
    // Read and print info about all records
    printf("\nAll records in file:\n");
    key = 0;
    while ((key = c_fstsui(iun, &ni, &nj, &nk)) >= 0) {
        status = c_fstprm(key, &dateo, &deet, &npas, &ni, &nj, &nk,
                         &nbits, &datyp, &ip1, &ip2, &ip3, typvar,
                         nomvar, etiket, grtyp, &ig1, &ig2, &ig3, &ig4,
                         &swa, &lng, &dltf, &ubc, &extra1, &extra2, &extra3);
        
        if (status >= 0) {
            printf("  Record: %.4s (%.2s) [%d x %d x %d] ip1=%d ip2=%d ip3=%d\n",
                   nomvar, typvar, ni, nj, nk, ip1, ip2, ip3);
        }
    }
    
    // Close file
    status = c_fstfrm(iun);
    if (status < 0) {
        printf("Failed to close FST file\n");
        c_fclos(iun);
        return 1;
    }
    
    status = c_fclos(iun);
    if (status < 0) {
        printf("Failed to close file\n");
        return 1;
    }
    printf("\nClosed file successfully\n");
    
    return 0;
} 
