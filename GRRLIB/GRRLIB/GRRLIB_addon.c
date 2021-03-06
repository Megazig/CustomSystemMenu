/*===========================================
        GRRLIB (GX version) 4.0.0 addon
        Code     : NoNameNo
        Additional Code : Crayon & Xane
        GX hints : RedShade
===========================================*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include "../lib/libpng/pngu/pngu.h"
#include "GRRLIB.h"
#include <fat.h>


extern u32 fb;
extern void *xfb[2];
extern GXRModeObj *rmode;

/**
 * Load a texture from a file.
 * @param filename The JPEG or PNG filename to load.
 * @return A GRRLIB_texImg structure filled with image informations.
 */
GRRLIB_texImg *GRRLIB_LoadTextureFromFile(const char *filename) {
    fatInitDefault();
    FILE *fd = fopen(filename, "rb");

    fseek(fd, 0, SEEK_END);
    long lsize = ftell(fd);
    rewind(fd);

    unsigned char *buffer = (unsigned char*) malloc (sizeof(unsigned char)*lsize);
    fread (buffer, 1, lsize, fd);
    GRRLIB_texImg *tex = GRRLIB_LoadTexture(buffer);
    free(buffer);

    fclose(fd);
    return tex;
}

/**
 * Make a PNG screenshot on the SD card.
 * libfat is required to use the function.
 * @param File name of the file to write.
 * @return true if every thing worked, false otherwise.
 */
bool GRRLIB_ScrShot(const char* File) {
    int ErrorCode = -1;
    IMGCTX pngContext;

    if(fatInitDefault() && (pngContext = PNGU_SelectImageFromDevice(File))) {
        ErrorCode = PNGU_EncodeFromYCbYCr(pngContext, rmode->fbWidth, rmode->efbHeight, xfb[fb], 0);
        PNGU_ReleaseImageContext(pngContext);
    }
    return !ErrorCode;
}

