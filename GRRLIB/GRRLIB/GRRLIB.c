/*===========================================
        GRRLIB (GX version) 4.0.0
        Code     : NoNameNo
        Additional Code : Crayon & Xane
        GX hints : RedShade
===========================================*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "../lib/libpng/pngu/pngu.h"
#include "../lib/libjpeg/jpeglib.h"
#include "GRRLIB.h"
#include <ogc/conf.h>


#define DEFAULT_FIFO_SIZE (256 * 1024) /**< GX fifo buffer size. */


u32 fb = 0;
void *xfb[2] = {NULL, NULL};
Mtx GXmodelView2D;
GXRModeObj *rmode;
void *gp_fifo = NULL;

// Declare Static Functions
static GRRLIB_drawSettings GRRLIB_Settings;
static void RawTo4x4RGBA(const unsigned char *src, void *dst, const unsigned int width, const unsigned int height);

// Declare Inline Functions
inline void GRRLIB_FillScreen(u32 color);
inline void GRRLIB_Plot(f32 x, f32 y, u32 color);
inline void GRRLIB_Line(f32 x1, f32 y1, f32 x2, f32 y2, u32 color);
inline void GRRLIB_Rectangle(f32 x, f32 y, f32 width, f32 height, u32 color, u8 filled);
inline void GRRLIB_Circle(f32 x, f32 y, f32 radius, u32 color, u8 filled);
inline void GRRLIB_DrawImg(f32 xpos, f32 ypos, struct GRRLIB_texImg *tex, float degrees, float scaleX, f32 scaleY, u32 color );
inline void GRRLIB_DrawImgQuad(Vector pos[4], struct GRRLIB_texImg *tex, u32 color);
inline void GRRLIB_DrawTile(f32 xpos, f32 ypos, struct GRRLIB_texImg *tex, float degrees, float scaleX, f32 scaleY, u32 color, int frame);
inline void GRRLIB_DrawTileQuad(Vector pos[4], struct GRRLIB_texImg *tex, u32 color,int frame);


/**
 * Turn AntiAliasing on/off.
 * @param aa Set to true to enable AntiAliasing (Default: Enabled).
 */
void GRRLIB_SetAntiAliasing(bool aa) {
    GRRLIB_Settings.antialias = aa;
}

/**
 * Get current AntiAliasing setting.
 * @return True if AntiAliasing is enabled.
 */
bool GRRLIB_GetAntiAliasing() {
    return GRRLIB_Settings.antialias;
}

/**
 * Set a blending mode.
 * @param blendmode The blending mode to use (Default: GRRLIB_BLEND_ALPHA).
 */
void GRRLIB_SetBlend( unsigned char blendmode ) {
    GRRLIB_Settings.blend = blendmode;
    switch (GRRLIB_Settings.blend) {
        case GRRLIB_BLEND_ALPHA:
            GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
            break;
        case GRRLIB_BLEND_ADD:
            GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_DSTALPHA, GX_LO_CLEAR);
            break;
        case GRRLIB_BLEND_SCREEN:
            GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCCLR, GX_BL_DSTALPHA, GX_LO_CLEAR);
            break;
        case GRRLIB_BLEND_MULTI:
            GX_SetBlendMode(GX_BM_SUBSTRACT, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
            break;
        case GRRLIB_BLEND_INV:
            GX_SetBlendMode(GX_BM_BLEND, GX_BL_INVSRCCLR, GX_BL_INVSRCCLR, GX_LO_CLEAR);
            break;
    }
}

/**
 * Get the current blending mode.
 * @return The current blending mode.
 */
unsigned char GRRLIB_GetBlend() {
    return GRRLIB_Settings.blend;
}

/**
 * Clear screen with a specific color.
 * @param color The color to use to fill the screen.
 */
inline void GRRLIB_FillScreen(u32 color) {
    GRRLIB_Rectangle(-40, -40, 680, 520, color, 1);
}

/**
 * Draw a dot.
 * @param x Specifies the x-coordinate of the dot.
 * @param y Specifies the y-coordinate of the dot.
 * @param color The color of the dot in RGBA format.
 */
inline void GRRLIB_Plot(f32 x, f32 y, u32 color) {
    Vector v[] = {{x,y,0.0f}};
    u32 ncolor[] = {color};

    GRRLIB_NPlot(v, ncolor, 1);
}

/**
 * Draw an array of points.
 * @param v Array containing the points.
 * @param color The color of the points in RGBA format.
 * @param n Number of points in the vector array.
 */
void GRRLIB_NPlot(Vector v[], u32 color[], long n) {
    GRRLIB_GXEngine(v, color, n, GX_POINTS);
}

/**
 * Draw a line.
 * @param x1 Starting point for line for the x coordinate.
 * @param y1 Starting point for line for the y coordinate.
 * @param x2 Ending point for line for the x coordinate.
 * @param y2 Ending point for line for the x coordinate.
 * @param color Line color in RGBA format.
 */
inline void GRRLIB_Line(f32 x1, f32 y1, f32 x2, f32 y2, u32 color) {
    Vector v[] = {{x1,y1,0.0f}, {x2,y2,0.0f}};
    u32 ncolor[] = {color,color};

    GRRLIB_NGone(v, ncolor, 2);
}

/**
 * Draw a rectangle.
 * @param x Specifies the x-coordinate of the upper-left corner of the rectangle.
 * @param y Specifies the y-coordinate of the upper-left corner of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param color The color of the rectangle in RGBA format.
 * @param filled Set to true to fill the rectangle.
 */
inline void GRRLIB_Rectangle(f32 x, f32 y, f32 width, f32 height, u32 color, u8 filled) {
    f32 x2 = x+width;
    f32 y2 = y+height;
    Vector v[] = {{x,y,0.0f}, {x2,y,0.0f}, {x2,y2,0.0f}, {x,y2,0.0f}, {x,y,0.0f}};
    u32 ncolor[]= {color,color,color,color,color};

    if (!filled) {
        GRRLIB_NGone(v, ncolor, 5);
    }
    else {
        GRRLIB_NGoneFilled(v, ncolor, 4);
    }
}

/**
 * Draw a circle.
 * @author Dark_Link
 * @param x Specifies the x-coordinate of the circle.
 * @param y Specifies the y-coordinate of the circle.
 * @param radius The radius of the circle.
 * @param color The color of the circle in RGBA format.
 * @param filled Set to true to fill the circle.
 */
inline void GRRLIB_Circle(f32 x, f32 y, f32 radius, u32 color, u8 filled) {
    Vector v[36];
    u32 ncolor[36];
    u32 a;
    f32 ra;
    f32 G_DTOR = M_DTOR * 10;

    for (a = 0; a < 36; a++) {
        ra = a * G_DTOR;

        v[a].x = cos(ra) * radius + x;
        v[a].y = sin(ra) * radius + y;
        v[a].z = 0.0f;
        ncolor[a]  = color;
    }

    if (!filled) {
        GRRLIB_GXEngine(v, ncolor, 36, GX_LINESTRIP);
    }
    else {
        GRRLIB_GXEngine(v, ncolor, 36, GX_TRIANGLEFAN);
    }
}

/**
 * Draw a polygon.
 * @param v The vector containing the coordinates of the polygon.
 * @param color The color of the filled polygon in RGBA format.
 * @param n Number of points in the vector.
 */
void GRRLIB_NGone(Vector v[], u32 color[], long n) {
    GRRLIB_GXEngine(v, color, n, GX_LINESTRIP);
}

/**
 * Draw a filled polygon.
 * @param v The vector containing the coordinates of the polygon.
 * @param color The color of the filled polygon in RGBA format.
 * @param n Number of points in the vector.
 */
void GRRLIB_NGoneFilled(Vector v[], u32 color[], long n) {
    GRRLIB_GXEngine(v, color, n, GX_TRIANGLEFAN);
}

/**
 * Convert a raw BMP (RGB, no alpha) to 4x4RGBA.
 * @author DragonMinded
 * @param src
 * @param dst
 * @param width
 * @param height
*/
static void RawTo4x4RGBA(const unsigned char *src, void *dst, const unsigned int width, const unsigned int height) {
    unsigned int block;
    unsigned int i;
    unsigned int c;
    unsigned int ar;
    unsigned int gb;
    unsigned char *p = (unsigned char*)dst;

    for (block = 0; block < height; block += 4) {
        for (i = 0; i < width; i += 4) {
            /* Alpha and Red */
            for (c = 0; c < 4; ++c) {
                for (ar = 0; ar < 4; ++ar) {
                    /* Alpha pixels */
                    *p++ = 255;
                    /* Red pixels */
                    *p++ = src[((i + ar) + ((block + c) * width)) * 3];
                }
            }

            /* Green and Blue */
            for (c = 0; c < 4; ++c) {
                for (gb = 0; gb < 4; ++gb) {
                    /* Green pixels */
                    *p++ = src[(((i + gb) + ((block + c) * width)) * 3) + 1];
                    /* Blue pixels */
                    *p++ = src[(((i + gb) + ((block + c) * width)) * 3) + 2];
                }
            }
        } /* i */
    } /* block */
}

/**
 * Initialize a tile set.
 * @param tex The texture to initialize.
 * @param tilew Width of the tile.
 * @param tileh Height of the tile.
 * @param tilestart Offset for starting position (Used in fonts).
 */
void GRRLIB_InitTileSet(struct GRRLIB_texImg *tex, unsigned int tilew, unsigned int tileh, unsigned int tilestart) {
    tex->tilew = tilew;
    tex->tileh = tileh;
    if (tilew) // Avoid division by zero
        tex->nbtilew = tex->w / tilew;
    if (tileh) // Avoid division by zero
        tex->nbtileh = tex->h / tileh;
    tex->tilestart = tilestart;
    tex->tiledtex = true;
    GRRLIB_SetHandle( tex, 0, 0 );
}

/**
 * Load a texture from a buffer.
 * @param my_png the PNG buffer to load.
 * @return A GRRLIB_texImg structure filled with image informations.
 */
GRRLIB_texImg *GRRLIB_LoadTexturePNG(const unsigned char my_png[]) {
    PNGUPROP imgProp;
    IMGCTX ctx;
    GRRLIB_texImg *my_texture = (struct GRRLIB_texImg *)calloc(1, sizeof(GRRLIB_texImg));

    if(my_texture != NULL) {
        ctx = PNGU_SelectImageFromBuffer(my_png);
        PNGU_GetImageProperties(ctx, &imgProp);
        my_texture->data = memalign(32, imgProp.imgWidth * imgProp.imgHeight * 4);
        if(my_texture->data != NULL) {
            PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, my_texture->data, 255);
            PNGU_ReleaseImageContext(ctx);
            my_texture->w = imgProp.imgWidth;
            my_texture->h = imgProp.imgHeight;
            GRRLIB_SetHandle( my_texture, 0, 0 );
            GRRLIB_FlushTex( my_texture );
        }
    }
    return my_texture;
}

/**
 * Load a texture from a buffer.
 * Take care to have the JPG finnish with 0xFF 0xD9!!
 * @author DrTwox
 * @param my_jpg The JPEG buffer to load.
 * @return A GRRLIB_texImg structure filled with image informations.
 */
GRRLIB_texImg *GRRLIB_LoadTextureJPG(const unsigned char my_jpg[]) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    GRRLIB_texImg *my_texture = (struct GRRLIB_texImg *)calloc(1, sizeof(GRRLIB_texImg));
    int n = 0;
    unsigned int i;

    if(my_texture == NULL)
        return NULL;

    if ((my_jpg[0]==0xFF) && (my_jpg[1]==0xD8) && (my_jpg[2]==0xFF)) {
        while(true) {
            if ((my_jpg[n]==0xFF) && (my_jpg[n+1]==0xD9))
                break;
            n++;
        }
        n+=2;
    }

    jpeg_create_decompress(&cinfo);
    cinfo.err = jpeg_std_error(&jerr);
    cinfo.progress = NULL;
    jpeg_memory_src(&cinfo, my_jpg, n);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    unsigned char *tempBuffer = (unsigned char*) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    JSAMPROW row_pointer[1];
    row_pointer[0] = (unsigned char*) malloc(cinfo.output_width * cinfo.num_components);
    size_t location = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++) {
            /* Put the decoded scanline into the tempBuffer */
            tempBuffer[ location++ ] = row_pointer[0][i];
        }
    }

    /* Create a buffer to hold the final texture */
    my_texture->data = memalign(32, cinfo.output_width * cinfo.output_height * 4);
    RawTo4x4RGBA(tempBuffer, my_texture->data, cinfo.output_width, cinfo.output_height);

    /* Done - Do cleanup and release allocated memory */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(tempBuffer);

    my_texture->w = cinfo.output_width;
    my_texture->h = cinfo.output_height;
    GRRLIB_SetHandle( my_texture, 0, 0 );
    GRRLIB_FlushTex( my_texture );
    return my_texture;
}

/**
 * Print formatted output.
 * @param xpos Specifies the x-coordinate of the upper-left corner of the text.
 * @param ypos Specifies the y-coordinate of the upper-left corner of the text.
 * @param bmf The ByteMap font to use.
 * @param zoom This is a factor by which the text size will be increase or decrease.
 * @param text Text to draw.
 * @param ... Optional arguments.
 */
void GRRLIB_PrintBMF(f32 xpos, f32 ypos, struct GRRLIB_bytemapFont *bmf, f32 zoom, const char *text, ...) {
    unsigned int i, j, x, y, n, size;
    char tmp[1024];

    va_list argp;
    va_start(argp, text);
    size = vsprintf(tmp, text, argp);
    va_end(argp);

    GRRLIB_texImg *tex_BMfont = GRRLIB_CreateEmptyTexture(640, 480);

    for (i=0; i<size; i++) {
        for (j=0; j<bmf->nbChar; j++) {
            if (tmp[i] == bmf->charDef[j].character) {
                n=0;
                for (y=0; y<bmf->charDef[j].height; y++) {
                    for (x=0; x<bmf->charDef[j].width; x++) {
                        if (bmf->charDef[j].data[n]) {
                            GRRLIB_SetPixelTotexImg(xpos + x + bmf->charDef[j].relx, ypos + y + bmf->charDef[j].rely,
                                tex_BMfont, bmf->palette[bmf->charDef[j].data[n]]);
                        }
                        n++;
                    }
                }
                xpos += bmf->charDef[j].shift + bmf->addSpace;
                break;
            }
        }
    }

    GRRLIB_FlushTex( tex_BMfont );
    GRRLIB_DrawImg(0, 0, tex_BMfont, 0, 1, 1, 0xFFFFFFFF);
    free(tex_BMfont->data);
    free(tex_BMfont);
}

/**
 * Load a ByteMap font structure from a buffer.
 * @param my_bmf The ByteMap font buffer to load.
 * @return A GRRLIB_bytemapFont structure filled with BMF informations.
 */
GRRLIB_bytemapFont *GRRLIB_LoadBMF(const unsigned char my_bmf[]) {
    GRRLIB_bytemapFont *fontArray = (struct GRRLIB_bytemapFont *)malloc(sizeof(GRRLIB_bytemapFont));
    int i, j = 1;
    u8 lineheight, usedcolors, highestcolor, nbPalette;
    short int sizeover, sizeunder, sizeinner, numcolpal;
    u16 nbPixels;

    if (fontArray != NULL && my_bmf[0]==0xE1 && my_bmf[1]==0xE6 && my_bmf[2]==0xD5 && my_bmf[3]==0x1A) {
        fontArray->version = my_bmf[4];
        lineheight = my_bmf[5];
        sizeover = my_bmf[6];
        sizeunder = my_bmf[7];
        fontArray->addSpace = my_bmf[8];
        sizeinner = my_bmf[9];
        usedcolors = my_bmf[10];
        highestcolor = my_bmf[11];
        nbPalette = my_bmf[16];
        numcolpal = 3 * nbPalette;
        fontArray->palette = (u32 *)calloc(nbPalette + 1, sizeof(u32));
        for (i=0; i < numcolpal; i+=3) {
            fontArray->palette[j++] = ((((my_bmf[i+17]<<2)+3)<<24) | (((my_bmf[i+18]<<2)+3)<<16) | (((my_bmf[i+19]<<2)+3)<<8) | 0xFF);
        }
        j = my_bmf[17 + numcolpal];
        fontArray->name = (char *)calloc(j + 1, sizeof(char));
        memcpy(fontArray->name, &my_bmf[18 + numcolpal], j);
        j = 18 + numcolpal + j;
        fontArray->nbChar = (my_bmf[j] | my_bmf[j+1]<<8);
        fontArray->charDef = (GRRLIB_bytemapChar *)calloc(fontArray->nbChar, sizeof(GRRLIB_bytemapChar));
        j++;
        for (i=0; i < fontArray->nbChar; i++) {
            fontArray->charDef[i].character = my_bmf[++j];
            fontArray->charDef[i].width = my_bmf[++j];
            fontArray->charDef[i].height = my_bmf[++j];
            fontArray->charDef[i].relx = my_bmf[++j];
            fontArray->charDef[i].rely = my_bmf[++j];
            fontArray->charDef[i].shift = my_bmf[++j];
            nbPixels = fontArray->charDef[i].width * fontArray->charDef[i].height;
            fontArray->charDef[i].data = (u8 *)malloc(nbPixels);
            if (nbPixels && fontArray->charDef[i].data) {
                memcpy(fontArray->charDef[i].data, &my_bmf[++j], nbPixels);
                j += (nbPixels - 1);
            }
        }
    }
    return fontArray;
}

/**
 * Free memory allocated by ByteMap fonts.
 * @param bmf A GRRLIB_bytemapFont structure.
 */
void GRRLIB_FreeBMF(GRRLIB_bytemapFont *bmf) {
    unsigned int i;

    for (i=0; i<bmf->nbChar; i++) {
        free(bmf->charDef[i].data);
    }
    free(bmf->charDef);
    free(bmf->palette);
    free(bmf->name);
}

/**
 * Load a texture from a buffer.
 * @param my_img The JPEG or PNG buffer to load.
 * @return A GRRLIB_texImg structure filled with image informations.
 */
GRRLIB_texImg *GRRLIB_LoadTexture(const unsigned char my_img[]) {
    if (my_img[0]==0xFF && my_img[1]==0xD8 && my_img[2]==0xFF) {
        return (GRRLIB_LoadTextureJPG(my_img));
    }
    else {
        return (GRRLIB_LoadTexturePNG(my_img));
    }
}

/**
 * Free memory allocated for texture.
 * @param tex A GRRLIB_texImg structure.
 */
void GRRLIB_FreeTexture(struct GRRLIB_texImg *tex) {
    free(tex->data);
    free(tex);
    tex = NULL;
}

/**
 * Create an empty texture.
 * @param w Width of the new texture to create.
 * @param h Height of the new texture to create.
 * @return A GRRLIB_texImg structure newly created.
 */
GRRLIB_texImg *GRRLIB_CreateEmptyTexture(unsigned int w, unsigned int h) {
    unsigned int x, y;
    GRRLIB_texImg *my_texture = (struct GRRLIB_texImg *)calloc(1, sizeof(GRRLIB_texImg));

    if(my_texture != NULL) {
        my_texture->data = memalign(32, h * w * 4);
        my_texture->w = w;
        my_texture->h = h;

        // Initialize the texture
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {
                GRRLIB_SetPixelTotexImg(x, y, my_texture, 0x00000000);
            }
        }
        GRRLIB_SetHandle( my_texture, 0, 0 );
        GRRLIB_FlushTex( my_texture );
    }
    return my_texture;
}


/**
 * Make a snapshot of the screen in a texture.
 * @return A pointer to a texture representing the screen or NULL if an error occurs.
 */
void GRRLIB_Screen2Texture(GRRLIB_texImg *tex) {
    if(tex->data != NULL) {
        GX_SetTexCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
        GX_SetTexCopyDst(rmode->fbWidth, rmode->efbHeight, GX_TF_RGBA8, GX_FALSE);
        GX_CopyTex(tex->data, GX_FALSE);
        GX_PixModeSync();
        GRRLIB_FlushTex(tex);
    }
}


/**
 * Draw a texture.
 * @param xpos Specifies the x-coordinate of the upper-left corner.
 * @param ypos Specifies the y-coordinate of the upper-left corner.
 * @param tex The texture to draw.
 * @param degrees Angle of rotation.
 * @param scaleX Specifies the x-coordinate scale. -1 could be used for flipping the texture horizontally.
 * @param scaleY Specifies the y-coordinate scale. -1 could be used for flipping the texture vertically.
 * @param color Color in RGBA format.
 */
inline void GRRLIB_DrawImg(f32 xpos, f32 ypos, struct GRRLIB_texImg *tex, float degrees, float scaleX, f32 scaleY, u32 color) {
    if (tex == NULL || tex->data == NULL) { return; }

    GXTexObj texObj;
    u16 width, height;
    Mtx m, m1, m2, mv;

    GX_InitTexObj(&texObj, tex->data, tex->w, tex->h, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    if (GRRLIB_Settings.antialias == false) {
        GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    }

    GX_LoadTexObj(&texObj, GX_TEXMAP0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    width = tex->w * 0.5;
    height = tex->h * 0.5;
    guMtxIdentity(m1);
    guMtxScaleApply(m1, m1, scaleX, scaleY, 1.0);
    Vector axis = (Vector) {0, 0, 1 };
    guMtxRotAxisDeg (m2, &axis, degrees);
    guMtxConcat(m2, m1, m);

    guMtxTransApply(m, m, xpos+width+tex->handlex-tex->offsetx+(scaleX*( -tex->handley*sin(-DegToRad(degrees)) - tex->handlex*cos(-DegToRad(degrees)) )), ypos+height+tex->handley-tex->offsety+(scaleX*( -tex->handley*cos(-DegToRad(degrees)) + tex->handlex*sin(-DegToRad(degrees)) )), 0);
    guMtxConcat(GXmodelView2D, m, mv);

    GX_LoadPosMtxImm(mv, GX_PNMTX0);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(-width, -height, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(0, 0);

        GX_Position3f32(width, -height, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(1, 0);

        GX_Position3f32(width, height, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(1, 1);

        GX_Position3f32(-width, height, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(0, 1);
    GX_End();
    GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

/**
 * Draw a textured quad.
 * @param pos Vector array of the 4 points.
 * @param tex The texture to draw.
 * @param color Color in RGBA format.
 */
inline void GRRLIB_DrawImgQuad(Vector pos[4], struct GRRLIB_texImg *tex, u32 color) {
    if (tex == NULL || tex->data == NULL) { return; }

    GXTexObj texObj;
    Mtx m, m1, m2, mv;

    GX_InitTexObj(&texObj, tex->data, tex->w, tex->h, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    if (GRRLIB_Settings.antialias == false) {
        GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    }

    GX_LoadTexObj(&texObj, GX_TEXMAP0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    guMtxIdentity(m1);
    guMtxScaleApply(m1, m1, 1, 1, 1.0);
    Vector axis = (Vector) {0, 0, 1 };
    guMtxRotAxisDeg (m2, &axis, 0);
    guMtxConcat(m2, m1, m);

    guMtxConcat(GXmodelView2D, m, mv);

    GX_LoadPosMtxImm(mv, GX_PNMTX0);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(pos[0].x, pos[0].y, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(0, 0);

        GX_Position3f32(pos[1].x, pos[1].y, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(1, 0);

        GX_Position3f32(pos[2].x, pos[2].y, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(1, 1);

        GX_Position3f32(pos[3].x, pos[3].y, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(0, 1);
    GX_End();
    GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

/**
 * Draw a tile.
 * @param xpos Specifies the x-coordinate of the upper-left corner.
 * @param ypos Specifies the y-coordinate of the upper-left corner.
 * @param tex The texture containing the tile to draw.
 * @param degrees Angle of rotation.
 * @param scaleX Specifies the x-coordinate scale. -1 could be used for flipping the texture horizontally.
 * @param scaleY Specifies the y-coordinate scale. -1 could be used for flipping the texture vertically.
 * @param color Color in RGBA format.
 * @param frame Specifies the frame to draw.
 */
inline void GRRLIB_DrawTile(f32 xpos, f32 ypos, struct GRRLIB_texImg *tex, float degrees, float scaleX, f32 scaleY, u32 color, int frame) {
    if (tex == NULL || tex->data == NULL) { return; }

    GXTexObj texObj;
    f32 width, height;
    Mtx m, m1, m2, mv;

    // Frame Correction by spiffen
    f32 FRAME_CORR = 0.001f;
    f32 s1 = (((frame%tex->nbtilew))/(f32)tex->nbtilew)+(FRAME_CORR/tex->w);
    f32 s2 = (((frame%tex->nbtilew)+1)/(f32)tex->nbtilew)-(FRAME_CORR/tex->w);
    f32 t1 = (((int)(frame/tex->nbtilew))/(f32)tex->nbtileh)+(FRAME_CORR/tex->h);
    f32 t2 = (((int)(frame/tex->nbtilew)+1)/(f32)tex->nbtileh)-(FRAME_CORR/tex->h);

    GX_InitTexObj(&texObj, tex->data, tex->tilew*tex->nbtilew, tex->tileh*tex->nbtileh, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    if (GRRLIB_Settings.antialias == false) {
        GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    }
    GX_LoadTexObj(&texObj, GX_TEXMAP0);

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    width = tex->tilew * 0.5f;
    height = tex->tileh * 0.5f;
    guMtxIdentity(m1);
    guMtxScaleApply(m1, m1, scaleX, scaleY, 1.0f);

    Vector axis = (Vector) {0, 0, 1 };
    guMtxRotAxisDeg(m2, &axis, degrees);
    guMtxConcat(m2, m1, m);
    guMtxTransApply(m, m, xpos+width+tex->handlex-tex->offsetx+(scaleX*( -tex->handley*sin(-DegToRad(degrees)) - tex->handlex*cos(-DegToRad(degrees)) )), ypos+height+tex->handley-tex->offsety+(scaleX*( -tex->handley*cos(-DegToRad(degrees)) + tex->handlex*sin(-DegToRad(degrees)) )), 0);
    guMtxConcat(GXmodelView2D, m, mv);

    GX_LoadPosMtxImm(mv, GX_PNMTX0);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(-width, -height, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(s1, t1);

        GX_Position3f32(width, -height,  0);
        GX_Color1u32(color);
        GX_TexCoord2f32(s2, t1);

        GX_Position3f32(width, height,  0);
        GX_Color1u32(color);
        GX_TexCoord2f32(s2, t2);

        GX_Position3f32(-width, height,  0);
        GX_Color1u32(color);
        GX_TexCoord2f32(s1, t2);
    GX_End();
    GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

/**
 * Draw a tile in a quad.
 * @param pos Vector array of the 4 points.
 * @param tex The texture to draw.
 * @param color Color in RGBA format.
 * @param frame Specifies the frame to draw.
 */

inline void GRRLIB_DrawTileQuad(Vector pos[4], struct GRRLIB_texImg *tex, u32 color, int frame) {
    if (tex == NULL || tex->data == NULL) { return; }

    GXTexObj texObj;
    Mtx m, m1, m2, mv;

    // Frame Correction by spiffen
    f32 FRAME_CORR = 0.001f;
    f32 s1 = (((frame%tex->nbtilew))/(f32)tex->nbtilew)+(FRAME_CORR/tex->w);
    f32 s2 = (((frame%tex->nbtilew)+1)/(f32)tex->nbtilew)-(FRAME_CORR/tex->w);
    f32 t1 = (((int)(frame/tex->nbtilew))/(f32)tex->nbtileh)+(FRAME_CORR/tex->h);
    f32 t2 = (((int)(frame/tex->nbtilew)+1)/(f32)tex->nbtileh)-(FRAME_CORR/tex->h);

    GX_InitTexObj(&texObj, tex->data, tex->tilew*tex->nbtilew, tex->tileh*tex->nbtileh, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    if (GRRLIB_Settings.antialias == false) {
        GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    }
    GX_LoadTexObj(&texObj, GX_TEXMAP0);

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    guMtxIdentity(m1);
    guMtxScaleApply(m1, m1, 1, 1, 1.0f);

    Vector axis = (Vector) {0, 0, 1 };
    guMtxRotAxisDeg(m2, &axis, 0);
    guMtxConcat(m2, m1, m);

    guMtxConcat(GXmodelView2D, m, mv);

    GX_LoadPosMtxImm(mv, GX_PNMTX0);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(pos[0].x, pos[0].y, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(s1, t1);

        GX_Position3f32(pos[1].x, pos[1].y, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(s2, t1);

        GX_Position3f32(pos[2].x, pos[2].y, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(s2, t2);

        GX_Position3f32(pos[3].x, pos[3].y, 0);
        GX_Color1u32(color);
        GX_TexCoord2f32(s1, t2);
    GX_End();
    GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

/**
 * Print formatted output.
 * @param xpos Specifies the x-coordinate of the upper-left corner of the text.
 * @param ypos Specifies the y-coordinate of the upper-left corner of the text.
 * @param tex The texture containing the character set.
 * @param color Text color in RGBA format. The alpha channel is used to change the opacity of the text.
 * @param zoom This is a factor by which the text size will be increase or decrease.
 * @param text Text to draw.
 * @param ... Optional arguments.
 */
void GRRLIB_Printf(f32 xpos, f32 ypos, struct GRRLIB_texImg *tex, u32 color, f32 zoom, const char *text, ...) {
    if (tex == NULL || tex->data == NULL) { return; }

    int i, size;
    char tmp[1024];

    va_list argp;
    va_start(argp, text);
    size = vsprintf(tmp, text, argp);
    va_end(argp);

    for (i = 0; i < size; i++) {
        u8 c = tmp[i]-tex->tilestart;
        GRRLIB_DrawTile(xpos+i*tex->tilew*zoom, ypos, tex, 0, zoom, zoom, color, c);
    }
}

/**
 * Determine whether the specified point lies within the specified rectangle.
 * @param hotx Specifies the x-coordinate of the upper-left corner of the rectangle.
 * @param hoty Specifies the y-coordinate of the upper-left corner of the rectangle.
 * @param hotw The width of the rectangle.
 * @param hoth The height of the rectangle.
 * @param wpadx Specifies the x-coordinate of the point.
 * @param wpady Specifies the y-coordinate of the point.
 * @return If the specified point lies within the rectangle, the return value is true otherwise it's false.
 */
bool GRRLIB_PtInRect(int hotx, int hoty, int hotw, int hoth, int wpadx, int wpady) {
    return(((wpadx>=hotx) & (wpadx<=(hotx+hotw))) & ((wpady>=hoty) & (wpady<=(hoty+hoth))));
}

/**
 * Determine whether a specified rectangle lies within another rectangle.
 * @param rect1x Specifies the x-coordinate of the upper-left corner of the rectangle.
 * @param rect1y Specifies the y-coordinate of the upper-left corner of the rectangle.
 * @param rect1w Specifies the width of the rectangle.
 * @param rect1h Specifies the height of the rectangle.
 * @param rect2x Specifies the x-coordinate of the upper-left corner of the rectangle.
 * @param rect2y Specifies the y-coordinate of the upper-left corner of the rectangle.
 * @param rect2w Specifies the width of the rectangle.
 * @param rect2h Specifies the height of the rectangle.
 * @return If the specified rectangle lies within the other rectangle, the return value is true otherwise it's false.
 */
bool GRRLIB_RectInRect(int rect1x, int rect1y, int rect1w, int rect1h, int rect2x, int rect2y, int rect2w, int rect2h) {
    return ((rect1x >= rect2x) && (rect1y >= rect2y) &&
        (rect1x+rect1w <= rect2x+rect2w) && (rect1y+rect1h <= rect2y+rect2h));
}

/**
 * Determine whether a part of a specified rectangle lies on another rectangle.
 * @param rect1x Specifies the x-coordinate of the upper-left corner of the first rectangle.
 * @param rect1y Specifies the y-coordinate of the upper-left corner of the first rectangle.
 * @param rect1w Specifies the width of the first rectangle.
 * @param rect1h Specifies the height of the first rectangle.
 * @param rect2x Specifies the x-coordinate of the upper-left corner of the second rectangle.
 * @param rect2y Specifies the y-coordinate of the upper-left corner of the second rectangle.
 * @param rect2w Specifies the width of the second rectangle.
 * @param rect2h Specifies the height of the second rectangle.
 * @return If the specified rectangle lies on the other rectangle, the return value is true otherwise it's false.
 */
bool GRRLIB_RectOnRect(int rect1x, int rect1y, int rect1w, int rect1h, int rect2x, int rect2y, int rect2w, int rect2h) {
    return (GRRLIB_PtInRect(rect1x, rect1y, rect1w, rect1h, rect2x, rect2y) ||
        GRRLIB_PtInRect(rect1x, rect1y, rect1w, rect1h, rect2x+rect2w, rect2y) ||
        GRRLIB_PtInRect(rect1x, rect1y, rect1w, rect1h, rect2x+rect2w, rect2y+rect2h) ||
        GRRLIB_PtInRect(rect1x, rect1y, rect1w, rect1h, rect2x, rect2y+rect2h));
}

/**
 * Clip the drawing area to an rectangle.
 * @param x The x-coordinate of the rectangle.
 * @param y The y-coordinate of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 */
void GRRLIB_ClipDrawing( int x, int y, int width, int height ) {
    GX_SetClipMode( GX_CLIP_ENABLE );
    GX_SetScissor( x, y, width, height );
}

/**
 * Reset the clipping to normal.
 */
void GRRLIB_ClipReset() {
    GX_SetClipMode( GX_CLIP_ENABLE );
    GX_SetScissor( 0, 0, rmode->fbWidth, rmode->efbHeight );
}

/**
 * Set a texture's X and Y handles. e.g. for rotation.
 * @param tex The texture to set the handle on.
 * @param x The x-coordinate of the handle.
 * @param y The y-coordinate of the handle.
 */
void GRRLIB_SetHandle( struct GRRLIB_texImg *tex, int x, int y ) {
    if (tex->tiledtex) {
        tex->handlex = -(((int)tex->tilew)/2) + x;
        tex->handley = -(((int)tex->tileh)/2) + y;
    } else {
        tex->handlex = -(((int)tex->w)/2) + x;
        tex->handley = -(((int)tex->h)/2) + y;
    }
}

/**
 * Center a texture's handles. e.g. for rotation.
 * @param tex The texture to center.
 * @param enabled
 */
void GRRLIB_SetMidHandle( struct GRRLIB_texImg *tex, bool enabled ) {
    if (enabled) {
        if (tex->tiledtex) {
            tex->offsetx = (((int)tex->tilew)/2);
            tex->offsety = (((int)tex->tileh)/2);
        } else {
            tex->offsetx = (((int)tex->w)/2);
            tex->offsety = (((int)tex->h)/2);
        }
        GRRLIB_SetHandle(tex, tex->offsetx, tex->offsety);
    } else {
        GRRLIB_SetHandle(tex, 0, 0);
        tex->offsetx = 0;
        tex->offsety = 0;
    }
}

/**
 * Return the color value of a pixel from a GRRLIB_texImg.
 * @param x Specifies the x-coordinate of the pixel in the texture.
 * @param y Specifies the y-coordinate of the pixel in the texture.
 * @param tex The texture to get the color from.
 * @return The color of a pixel in RGBA format.
 */
u32 GRRLIB_GetPixelFromtexImg(int x, int y, struct GRRLIB_texImg *tex) {
    u8 *truc = (u8*)tex->data;
    u8 r, g, b, a;
    u32 offset;

    offset = (((y >> 2)<<4)*tex->w) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) << 1); // Fuckin equation found by NoNameNo ;)

    a=*(truc+offset);
    r=*(truc+offset+1);
    g=*(truc+offset+32);
    b=*(truc+offset+33);

    return ((r<<24) | (g<<16) | (b<<8) | a);
}

/**
 * Set the color value of a pixel to a GRRLIB_texImg.
 * @see GRRLIB_FlushTex
 * @param x Specifies the x-coordinate of the pixel in the texture.
 * @param y Specifies the y-coordinate of the pixel in the texture.
 * @param tex The texture to set the color to.
 * @param color The color of the pixel in RGBA format.
 */
void GRRLIB_SetPixelTotexImg(int x, int y, GRRLIB_texImg *tex, u32 color) {
    u8 *truc = (u8*)tex->data;
    u32 offset;

    offset = (((y >> 2)<<4)*tex->w) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) <<1); // Fuckin equation found by NoNameNo ;)

    *(truc+offset)=color & 0xFF;
    *(truc+offset+1)=(color>>24) & 0xFF;
    *(truc+offset+32)=(color>>16) & 0xFF;
    *(truc+offset+33)=(color>>8) & 0xFF;
}

/**
 * Write the contents of a texture in the data cache down to main memory.
 * For performance the CPU holds a data cache where modifications are stored before they get written down to mainmemory.
 * @param tex The texture to flush.
 */
void GRRLIB_FlushTex(struct GRRLIB_texImg *tex) {
    DCFlushRange(tex->data, tex->w * tex->h * 4);
}

/**
 * Change a texture to gray scale.
 * @see GRRLIB_FlushTex
 * @param texsrc The texture source.
 * @param texdest The texture grayscaled destination.
 */
void GRRLIB_BMFX_Grayscale(struct GRRLIB_texImg *texsrc, GRRLIB_texImg *texdest) {
    unsigned int x, y;
    u8 gray;
    u32 color;

    for (y = 0; y < texsrc->h; y++) {
        for (x = 0; x < texsrc->w; x++) {
            color = GRRLIB_GetPixelFromtexImg(x, y, texsrc);

            gray = (((color >> 24 & 0xFF)*77 + (color >> 16 & 0xFF)*150 + (color >> 8 & 0xFF)*28) / (255));

            GRRLIB_SetPixelTotexImg(x, y, texdest,
                ((gray << 24) | (gray << 16) | (gray << 8) | (color & 0xFF)));
        }
    }
    GRRLIB_SetHandle( texdest, 0, 0 );
}

/**
 * Invert colors of the texture.
 * @see GRRLIB_FlushTex
 * @param texsrc The texture source.
 * @param texdest The texture destination.
 */
void GRRLIB_BMFX_Invert(struct GRRLIB_texImg *texsrc, GRRLIB_texImg *texdest) {
    unsigned int x, y;
    u32 color;

    for (y = 0; y < texsrc->h; y++) {
        for (x = 0; x < texsrc->w; x++) {
            color = GRRLIB_GetPixelFromtexImg(x, y, texsrc);
            GRRLIB_SetPixelTotexImg(x, y, texdest,
                ((0xFFFFFF - (color >> 8 & 0xFFFFFF)) << 8)  | (color & 0xFF));
        }
    }
}

/**
 * Flip texture horizontal.
 * @see GRRLIB_FlushTex
 * @param texsrc The texture source.
 * @param texdest The texture destination.
 */
void GRRLIB_BMFX_FlipH(struct GRRLIB_texImg *texsrc, GRRLIB_texImg *texdest) {
    unsigned int x, y, txtWidth = texsrc->w - 1;

    for (y = 0; y < texsrc->h; y++) {
        for (x = 0; x < texsrc->w; x++) {
            GRRLIB_SetPixelTotexImg(txtWidth - x, y, texdest,
                GRRLIB_GetPixelFromtexImg(x, y, texsrc));
        }
    }
}

/**
 * Flip texture vertical.
 * @see GRRLIB_FlushTex
 * @param texsrc The texture source.
 * @param texdest The texture destination.
 */
void GRRLIB_BMFX_FlipV(struct GRRLIB_texImg *texsrc, GRRLIB_texImg *texdest) {
    unsigned int x, y, texHeight = texsrc->h - 1;

    for (y = 0; y < texsrc->h; y++) {
        for (x = 0; x < texsrc->w; x++) {
            GRRLIB_SetPixelTotexImg(x, texHeight - y, texdest,
                GRRLIB_GetPixelFromtexImg(x, y, texsrc));
        }
    }
}

/**
 * A texture effect (Blur).
 * @see GRRLIB_FlushTex
 * @param texsrc The texture source.
 * @param texdest The texture destination.
 * @param factor The blur factor.
 */
void GRRLIB_BMFX_Blur(struct GRRLIB_texImg *texsrc, GRRLIB_texImg *texdest, u32 factor) {
    int numba = (1+(factor<<1))*(1+(factor<<1));
    u32 x, y;
    s32 k, l;
    int tmp = 0;
    int newr, newg, newb, newa;
    u32 colours[numba];
    u32 thiscol;

    for (x = 0; x < texsrc->w; x++) {
        for (y = 0; y < texsrc->h; y++) {
            newr = 0;
            newg = 0;
            newb = 0;
            newa = 0;

            tmp = 0;
            thiscol = GRRLIB_GetPixelFromtexImg(x, y, texsrc);

            for (k = x - factor; k <= x + factor; k++) {
                for (l = y - factor; l <= y + factor; l++) {
                    if (k < 0) { colours[tmp] = thiscol; }
                    else if (k >= texsrc->w) { colours[tmp] = thiscol; }
                    else if (l < 0) { colours[tmp] = thiscol; }
                    else if (l >= texsrc->h) { colours[tmp] = thiscol; }
                    else { colours[tmp] = GRRLIB_GetPixelFromtexImg(k, l, texsrc); }
                    tmp++;
                }
            }

            for (tmp = 0; tmp < numba; tmp++) {
                newr += (colours[tmp] >> 24) & 0xFF;
                newg += (colours[tmp] >> 16) & 0xFF;
                newb += (colours[tmp] >> 8) & 0xFF;
                newa += colours[tmp] & 0xFF;
            }

            newr /= numba;
            newg /= numba;
            newb /= numba;
            newa /= numba;

            GRRLIB_SetPixelTotexImg(x, y, texdest, (newr<<24) | (newg<<16) | (newb<<8) | newa);
        }
    }
}

/**
 * A texture effect (Pixelate).
 * @see GRRLIB_FlushTex
 * @param texsrc The texture source.
 * @param texdest The texture destination.
 * @param factor The factor level of the effect.
 */
void GRRLIB_BMFX_Pixelate(struct GRRLIB_texImg *texsrc, GRRLIB_texImg *texdest, u32 factor) {
    unsigned int x, y;
    unsigned int xx, yy;
    u32 rgb;

    for (x = 0; x < texsrc->w - 1 - factor; x += factor) {
        for (y = 0; y < texsrc->h - 1 - factor; y +=factor) {
            rgb = GRRLIB_GetPixelFromtexImg(x, y, texsrc);
                for (xx = x; xx < x + factor; xx++) {
                    for (yy = y; yy < y + factor; yy++) {
                        GRRLIB_SetPixelTotexImg(xx, yy, texdest, rgb);
                    }
                }
        }
    }
}

/**
 * A texture effect (Scatter).
 * @see GRRLIB_FlushTex
 * @param texsrc The texture source.
 * @param texdest The texture destination.
 * @param factor The factor level of the effect.
 */
void GRRLIB_BMFX_Scatter(struct GRRLIB_texImg *texsrc, GRRLIB_texImg *texdest, u32 factor) {
    unsigned int x, y;
    u32 val1, val2;
    u32 val3, val4;
    int factorx2 = factor*2;

    for (y = 0; y < texsrc->h; y++) {
        for (x = 0; x < texsrc->w; x++) {
            val1 = x + (int) (factorx2 * (rand() / (RAND_MAX + 1.0))) - factor;
            val2 = y + (int) (factorx2 * (rand() / (RAND_MAX + 1.0))) - factor;

            if ((val1 >= texsrc->w) || (val2 >= texsrc->h)) {
            }
            else {
                val3 = GRRLIB_GetPixelFromtexImg(x, y, texsrc);
                val4 = GRRLIB_GetPixelFromtexImg(val1, val2, texsrc);
                GRRLIB_SetPixelTotexImg(x, y, texdest, val4);
                GRRLIB_SetPixelTotexImg(val1, val2, texdest, val3);
            }
        }
    }
}

/**
 * Draws a vector.
 */
void GRRLIB_GXEngine(Vector v[], u32 color[], long n, u8 fmt) {
    int i;

    GX_Begin(fmt, GX_VTXFMT0, n);
    for (i = 0; i < n; i++) {
        GX_Position3f32(v[i].x, v[i].y,  v[i].z);
        GX_Color1u32(color[i]);
    }
    GX_End();
}

/**
 * Initialize GRRLIB. Call this at the beginning your code.
 * @see GRRLIB_Exit
 */
void GRRLIB_Init() {
    f32 yscale;
    u32 xfbHeight;
    Mtx44 perspective;

    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    if (rmode == NULL)
        return;

	// Video Mode Correction
    switch (rmode->viTVMode) {
    	case VI_DEBUG_PAL:	// PAL 50hz 576i
    		rmode = &TVPal574IntDfScale;
    		break;
    }

    // Widescreen patch by CashMan's Productions (http://www.CashMan-Productions.fr.nf)
    if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
        rmode->viWidth = 678;
        rmode->viXOrigin = (VI_MAX_WIDTH_NTSC - 678)/2;
    }

    VIDEO_Configure(rmode);
    xfb[0] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    xfb[1] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    if (xfb[0] == NULL || xfb[1] == NULL)
        return;

    VIDEO_SetNextFramebuffer(xfb[fb]);
    VIDEO_SetBlack(true);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode&VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    gp_fifo = (u8 *) memalign(32, DEFAULT_FIFO_SIZE);
    if (gp_fifo == NULL)
        return;
    memset(gp_fifo, 0, DEFAULT_FIFO_SIZE);
    GX_Init(gp_fifo, DEFAULT_FIFO_SIZE);

    // Clears the BG to color and clears the z-buffer
    GX_SetCopyClear((GXColor){ 0, 0, 0, 0xff }, GX_MAX_Z24);

    // Other GX setup
    yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

    if (rmode->aa) {
        // Set 16 bit RGB565
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    } else {
        // Set 24 bit Z24
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    }

    GX_SetDispCopyGamma(GX_GM_1_0);

    // Setup the vertex descriptor
    // Tells the flipper to expect direct data
    GX_ClearVtxDesc();
    GX_InvVtxCache();
    GX_InvalidateTexAll();

    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
    GX_SetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,   GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_TRUE);

    GX_SetNumChans(1);
    GX_SetNumTexGens(1);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    guMtxIdentity(GXmodelView2D);
    guMtxTransApply(GXmodelView2D, GXmodelView2D, 0.0F, 0.0F, -100.0F);
    GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);

    guOrtho(perspective, 0, rmode->efbHeight, 0, rmode->fbWidth, 0, 1000.0f);
    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetAlphaUpdate(GX_TRUE);
    GX_SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GX_SetColorUpdate(GX_ENABLE);
    GX_SetCullMode(GX_CULL_NONE);
    GRRLIB_ClipReset();

    VIDEO_SetBlack(false);

    // Default settings
    GRRLIB_Settings.antialias = true;
    GRRLIB_Settings.blend = GRRLIB_BLEND_ALPHA;
}

/**
 * Call this function after drawing.
 */
void GRRLIB_Render() {
    GX_DrawDone();
    GX_InvalidateTexAll();

    fb ^= 1;        // Flip framebuffer
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(xfb[fb], GX_TRUE);

    VIDEO_SetNextFramebuffer(xfb[fb]);
    VIDEO_Flush();
    VIDEO_WaitVSync();
}

/**
 * Call this before exiting your application.
 */
void GRRLIB_Exit() {
    GX_SetClipMode( GX_CLIP_DISABLE );
    GX_SetScissor( 0, 0, rmode->fbWidth, rmode->efbHeight );
    GRRLIB_FillScreen( 0x000000FF );
    GRRLIB_Render();
    GX_DrawDone();
    GX_AbortFrame();

    if (xfb[0] != NULL) {
        free(MEM_K1_TO_K0(xfb[0]));
        xfb[0] = NULL;
    }
    if (xfb[1] != NULL) {
        free(MEM_K1_TO_K0(xfb[1]));
        xfb[1] = NULL;
    }
    if (gp_fifo != NULL) {
        free(gp_fifo);
        gp_fifo = NULL;
    }
}

/**
 * Reads a pixel directly from the FrontBuffer.
 * Since the FB is stored in YCbCr,
 * @param x The x-coordinate within the FB.
 * @param y The y-coordinate within the FB.
 * @param R1 A pointer to a variable receiving the first Red value.
 * @param G1 A pointer to a variable receiving the first Green value.
 * @param B1 A pointer to a variable receiving the first Blue value.
 * @param R2 A pointer to a variable receiving the second Red value.
 * @param G2 A pointer to a variable receiving the second Green value.
 * @param B2 A pointer to a variable receiving the second Blue value.
 */
void GRRLIB_GetPixelFromFB(int x, int y, u8 *R1, u8 *G1, u8 *B1, u8* R2, u8 *G2, u8 *B2 ) {
    // Position Correction
    if (x > (rmode->fbWidth/2)) { x = (rmode->fbWidth/2); }
    if (x < 0) { x = 0; }
    if (y > rmode->efbHeight) { y = rmode->efbHeight; }
    if (y < 0) { y = 0; }

    // Preparing FB for reading
    u32 Buffer = (((u32 *)xfb[fb])[y*(rmode->fbWidth/2)+x]);
    u8 *Colors = (u8 *) &Buffer;

    /** Color channel:
    Colors[0] = Y1
    Colors[1] = Cb
    Colors[2] = Y2
    Colors[3] = Cr */

    *R1 = GRRLIB_ClampVar8( 1.164 * (Colors[0] - 16) + 1.596 * (Colors[3] - 128) );
    *G1 = GRRLIB_ClampVar8( 1.164 * (Colors[0] - 16) - 0.813 * (Colors[3] - 128) - 0.392 * (Colors[1] - 128) );
    *B1 = GRRLIB_ClampVar8( 1.164 * (Colors[0] - 16) + 2.017 * (Colors[1] - 128) );

    *R2 = GRRLIB_ClampVar8( 1.164 * (Colors[2] - 16) + 1.596 * (Colors[3] - 128) );
    *G2 = GRRLIB_ClampVar8( 1.164 * (Colors[2] - 16) - 0.813 * (Colors[3] - 128) - 0.392 * (Colors[1] - 128) );
    *B2 = GRRLIB_ClampVar8( 1.164 * (Colors[2] - 16) + 2.017 * (Colors[1] - 128) );
}


/**
 * A helper function for the YCbCr -> RGB conversion.
 * Clamps the given value into a range of 0 - 255 and thus preventing an overflow.
 * @param Value The value to clamp.
 * @return Returns a clean, clamped unsigned char.
 */
u8 GRRLIB_ClampVar8(float Value) {
    /* Using float to increase the precision.
    This makes a full spectrum (0 - 255) possible. */
    Value = roundf(Value);
    if (Value < 0) {
        Value = 0;
    } else if (Value > 255) {
        Value = 255;
    }
    return (u8)Value;
}

/**
 * Converts RGBA values to u32 color.
 * @param r Amount of Red (0 - 255);
 * @param g Amount of Green (0 - 255);
 * @param b Amount of Blue (0 - 255);
 * @param a Amount of Alpha (0 - 255);
 * @return Returns the color in u32 format.
 */
u32 GRRLIB_GetColor( u8 r, u8 g, u8 b, u8 a ) {
    return (r << 24) | (g << 16) | (b << 8) | a;
}
