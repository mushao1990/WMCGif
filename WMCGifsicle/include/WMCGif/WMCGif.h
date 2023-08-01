//
//  WMCGif.h
//  WMCGif
//
//  Created by muser on 2023/08/01.
//  Copyright © 2023 Mac. All rights reserved.
//

#ifndef WMC_GIF_H /* -*- mode: c -*- */
#define WMC_GIF_H
#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct WMC_Stream       WMC_Stream;
typedef struct WMC_Image        WMC_Image;
typedef struct WMC_Colormap     WMC_Colormap;
typedef struct WMC_Comment      WMC_Comment;
typedef struct WMC_Extension    WMC_Extension;
typedef struct WMC_Record       WMC_Record;

typedef uint16_t WMC_Code;
#define WMC_GIF_MAX_CODE_BITS       12
#define WMC_GIF_MAX_CODE            0x1000
#define WMC_GIF_MAX_BLOCK           255


/** GIF_STREAM **/

struct WMC_Stream {
    WMC_Image **images;
    int nimages;
    int imagescap;

    WMC_Colormap *global;
    uint16_t background;        /* 256 means no background */

    uint16_t screen_width;
    uint16_t screen_height;
    long loopcount;             /* -1 means no loop count */

    WMC_Comment* end_comment;
    WMC_Extension* end_extension_list;

    unsigned errors;

    int userflags;
    const char* landmark;
    int refcount;
};

WMC_Stream *    WMC_NewStream(void);
void            WMC_DeleteStream(WMC_Stream *);

WMC_Stream *    WMC_CopyStreamSkeleton(WMC_Stream *);
WMC_Stream *    WMC_CopyStreamImages(WMC_Stream *);

void            WMC_CalculateScreenSize(WMC_Stream *, int force);
int             WMC_Unoptimize(WMC_Stream *);
int             WMC_FullUnoptimize(WMC_Stream *, int flags);


/** GIF_IMAGE **/

struct WMC_Image {
    uint16_t width;
    uint16_t height;

    uint8_t **img;              /* img[y][x] == image byte (x,y) */
    uint8_t *image_data;

    uint16_t left;
    uint16_t top;
    uint16_t delay;
    uint8_t disposal;
    uint8_t interlace;

    WMC_Colormap *local;
    short transparent;          /* -1 means no transparent index */

    uint16_t user_flags;

    char *identifier;
    WMC_Comment* comment;
    WMC_Extension* extension_list;

    void (*free_image_data)(void *);

    uint32_t compressed_len;
    uint8_t *compressed;
    void (*free_compressed)(void *);

    void *user_data;
    void (*free_user_data)(void *);
    int refcount;

};

#define         WMC_GIF_DISPOSAL_NONE               0

WMC_Image *     WMC_NewImage(void);
void            WMC_DeleteImage(WMC_Image *gfi);

int             WMC_AddImage(WMC_Stream *gfs, WMC_Image *gfi);
void            WMC_RemoveImage(WMC_Stream *gfs, int i);
WMC_Image *     WMC_CopyImage(WMC_Image *gfi);
void            WMC_MakeImageEmpty(WMC_Image* gfi);

WMC_Image *     WMC_GetImage(WMC_Stream *gfs, int i);
WMC_Image *     WMC_GetNamedImage(WMC_Stream *gfs, const char *name);
int             WMC_ImageNumber(WMC_Stream *gfs, WMC_Image *gfi);

#define         WMC_ImageWidth(gfi)             ((gfi)->width)
#define         WMC_ImageHeight(gfi)            ((gfi)->height)
#define         WMC_ImageDelay(gfi)             ((gfi)->delay)
#define         WMC_ImageUserData(gfi)          ((gfi)->userdata)
#define         WMC_SetImageUserData(gfi, v)    ((gfi)->userdata = v)
int             WMC_ImageColorBound(const WMC_Image* gfi);

typedef         void (*WMC_ReadErrorHandler)(WMC_Stream* gfs,
                                             WMC_Image* gfi,
                                             int is_error,
                                             const char* error_text);

typedef struct {
    int flags;
    int loss;
    void *padding[7];
} WMC_CompressInfo;

#define         WMC_UncompressImage(gfs, gfi) WMC_FullUncompressImage((gfs),(gfi),0)
int             WMC_FullUncompressImage(WMC_Stream* gfs, WMC_Image* gfi,
                                        WMC_ReadErrorHandler handler);
int             WMC_CompressImage(WMC_Stream *gfs, WMC_Image *gfi);
int             WMC_FullCompressImage(WMC_Stream *gfs, WMC_Image *gfi,
                                      const WMC_CompressInfo *gcinfo);
void            WMC_ReleaseUncompressedImage(WMC_Image *gfi);
void            WMC_ReleaseCompressedImage(WMC_Image *gfi);
int             WMC_SetUncompressedImage(WMC_Image *gfi, uint8_t *data,
                        void (*free_data)(void *), int data_interlaced);
int             WMC_CreateUncompressedImage(WMC_Image* gfi, int data_interlaced);

//int             WMC_ClipImage(WMC_Image *gfi, int l, int t, int w, int h);

void            WMC_InitCompressInfo(WMC_CompressInfo *gcinfo);


/** GIF_COLORMAP **/

typedef struct {
    uint8_t haspixel;      /* semantics assigned by user */
    uint8_t gfc_red;       /* red component (0-255) */
    uint8_t gfc_green;     /* green component (0-255) */
    uint8_t gfc_blue;      /* blue component (0-255) */
    uint32_t pixel;        /* semantics assigned by user */
} WMC_Color;


struct WMC_Colormap {
    int ncol;
    int capacity;
    uint32_t userflags;
    int refcount;
    WMC_Color *col;
};

WMC_Colormap *  WMC_NewColormap(void);
WMC_Colormap *  WMC_NewFullColormap(int count, int capacity);
void            WMC_DeleteColormap(WMC_Colormap *);

WMC_Colormap *  WMC_CopyColormap(WMC_Colormap *);

int             WMC_ColorEq(WMC_Color *, WMC_Color *);
#define         GIF_COLOREQ(c1, c2) \
((c1)->gfc_red==(c2)->gfc_red && (c1)->gfc_green==(c2)->gfc_green && \
 (c1)->gfc_blue==(c2)->gfc_blue)
#define         GIF_SETCOLOR(c, r, g, b) \
((c)->gfc_red = (r), (c)->gfc_green = (g), (c)->gfc_blue = (b))

int             WMC_FindColor(WMC_Colormap *, WMC_Color *);
int             WMC_AddColor(WMC_Colormap *, WMC_Color *, int look_from);


/** GIF_COMMENT **/

struct WMC_Comment {
    char **str;
    int *len;
    int count;
    int cap;
};

WMC_Comment *   WMC_NewComment(void);
void            WMC_DeleteComment(WMC_Comment *);
int             WMC_AddCommentTake(WMC_Comment *, char *, int);
int             WMC_AddComment(WMC_Comment *, const char *, int);


/** GIF_EXTENSION **/

struct WMC_Extension {
    int kind;                   /* negative kinds are reserved */
    char* appname;
    int applength;
    uint8_t* data;
    uint32_t length;
    int packetized;

    WMC_Stream *stream;
    WMC_Image *image;
    WMC_Extension *next;
    void (*free_data)(void *);
};


WMC_Extension*  WMC_NewExtension(int kind, const char* appname, int applength);
void            WMC_DeleteExtension(WMC_Extension* gfex);
WMC_Extension*  WMC_CopyExtension(WMC_Extension* gfex);
int             WMC_AddExtension(WMC_Stream* gfs, WMC_Image* gfi,
                                 WMC_Extension* gfex);


/** READING AND WRITING **/

struct WMC_Record {
    const unsigned char *data;
    uint32_t length;
};

#define WMC_GIF_READ_COMPRESSED             1
#define WMC_GIF_READ_UNCOMPRESSED           2
#define WMC_GIF_READ_CONST_RECORD           4
#define WMC_GIF_READ_TRAILING_GARBAGE_OK    8
#define WMC_GIF_WRITE_CAREFUL_MIN_CODE_SIZE 1
#define WMC_GIF_WRITE_EAGER_CLEAR           2
#define WMC_GIF_WRITE_OPTIMIZE              4
#define WMC_GIF_WRITE_SHRINK                8

void            WMC_SetErrorHandler(WMC_ReadErrorHandler handler);
WMC_Stream*     WMC_ReadFile(FILE* f);
WMC_Stream*     WMC_FullReadFile(FILE* f, int flags, const char* landmark,
                                 WMC_ReadErrorHandler handler);
WMC_Stream*     WMC_ReadRecord(const WMC_Record* record);
WMC_Stream*     WMC_FullReadRecord(const WMC_Record* record, int flags,
                                   const char* landmark,
                                   WMC_ReadErrorHandler handler);
int             WMC_WriteFile(WMC_Stream *gfs, FILE *f);
int             WMC_FullWriteFile(WMC_Stream *gfs,
                                  const WMC_CompressInfo *gcinfo, FILE *f);

#define WMC_ReadFile(f)         WMC_FullReadFile((f),WMC_GIF_READ_UNCOMPRESSED,0,0)
#define WMC_ReadRecord(r)       WMC_FullReadRecord((r),WMC_GIF_READ_UNCOMPRESSED,0,0)
#define WMC_CompressImage(s, i) WMC_FullCompressImage((s),(i),0)
#define WMC_WriteFile(s, f)     WMC_FullWriteFile((s),0,(f))

typedef struct WMC_Writer WMC_Writer;
WMC_Writer*     WMC_IncrementalWriteFileInit(WMC_Stream* gfs, const WMC_CompressInfo* gcinfo, FILE *f);
int             WMC_IncrementalWriteImage(WMC_Writer* grr, WMC_Stream* gfs, WMC_Image* gfi);
int             WMC_IncrementalWriteComplete(WMC_Writer* grr, WMC_Stream* gfs);


/** HOOKS AND MISCELLANEOUS **/

int             WMC_InterlaceLine(int y, int height);
char *          WMC_CopyString(const char *);

#define GIF_T_STREAM                    (0)
#define GIF_T_IMAGE                     (1)
#define GIF_T_COLORMAP                  (2)
typedef void    (*WMC_DeletionHookFunc)(int, void *, void *);
int             WMC_AddDeletionHook(int, WMC_DeletionHookFunc, void *);
void            WMC_RemoveDeletionHook(int, WMC_DeletionHookFunc, void *);

#ifdef GIF_DEBUGGING
#define         GIF_DEBUG(x)                    WMC_Debug x
void            WMC_Debug(char *x, ...);
#else
#define         GIF_DEBUG(x)
#endif

void*           WMC_Realloc(void* p, size_t s, size_t n,
                            const char* file, int line);
void            WMC_Free(void* p);
#if !GIF_ALLOCATOR_DEFINED
# define        WMC_Free        free
#endif

#ifndef WMC_New
# define WMC_New(t)             ((t*) WMC_Realloc(0, sizeof(t), 1, __FILE__, __LINE__))
# define WMC_NewArray(t, n)     ((t*) WMC_Realloc(0, sizeof(t), (n), __FILE__, __LINE__))
# define WMC_ReArray(p, t, n)   ((p)=(t*) WMC_Realloc((void*) (p), sizeof(t), (n), __FILE__, __LINE__))
# define WMC_Delete(p)          WMC_Free((void*) (p))
# define WMC_DeleteArray(p)     WMC_Free((void*) (p))
#endif

#ifdef __cplusplus
}
#endif
#endif
