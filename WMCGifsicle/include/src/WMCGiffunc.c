//
//  WMCGif.h
//  WMCGif
//
//  Created by muser on 2023/08/01.
//  Copyright © 2023 Mac. All rights reserved.
//

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include "WMCGif.h"
#include <string.h>
#include <stdarg.h>
#ifdef __cplusplus
extern "C" {
#endif


WMC_Stream *
WMC_NewStream(void)
{
  WMC_Stream *gfs = WMC_New(WMC_Stream);
  if (!gfs)
    return 0;
  gfs->images = 0;
  gfs->nimages = gfs->imagescap = 0;
  gfs->global = 0;
  gfs->background = 256;
  gfs->screen_width = gfs->screen_height = 0;
  gfs->loopcount = -1;
  gfs->end_comment = 0;
  gfs->end_extension_list = 0;
  gfs->errors = 0;
  gfs->userflags = 0;
  gfs->refcount = 0;
  gfs->landmark = 0;
  return gfs;
}


WMC_Image *
WMC_NewImage(void)
{
  WMC_Image *gfi = WMC_New(WMC_Image);
  if (!gfi)
    return 0;
  gfi->width = gfi->height = 0;
  gfi->img = 0;
  gfi->image_data = 0;
  gfi->left = gfi->top = 0;
  gfi->delay = 0;
  gfi->disposal = WMC_GIF_DISPOSAL_NONE;
  gfi->interlace = 0;
  gfi->local = 0;
  gfi->transparent = -1;
  gfi->user_flags = 0;
  gfi->identifier = 0;
  gfi->comment = 0;
  gfi->extension_list = 0;
  gfi->free_image_data = WMC_Free;
  gfi->compressed_len = 0;
  gfi->compressed = 0;
  gfi->free_compressed = 0;
  gfi->user_data = 0;
  gfi->free_user_data = 0;
  gfi->refcount = 0;
  return gfi;
}


WMC_Colormap *
WMC_NewColormap(void)
{
  WMC_Colormap *gfcm = WMC_New(WMC_Colormap);
  if (!gfcm)
    return 0;
  gfcm->ncol = 0;
  gfcm->capacity = 0;
  gfcm->col = 0;
  gfcm->refcount = 0;
  gfcm->userflags = 0;
  return gfcm;
}


WMC_Colormap *
WMC_NewFullColormap(int count, int capacity)
{
  WMC_Colormap *gfcm = WMC_New(WMC_Colormap);
  if (!gfcm || capacity <= 0 || count < 0) {
    WMC_Delete(gfcm);
    return 0;
  }
  if (count > capacity)
    capacity = count;
  gfcm->ncol = count;
  gfcm->capacity = capacity;
  gfcm->col = WMC_NewArray(WMC_Color, capacity);
  gfcm->refcount = 0;
  gfcm->userflags = 0;
  if (!gfcm->col) {
    WMC_Delete(gfcm);
    return 0;
  } else
    return gfcm;
}


WMC_Comment *
WMC_NewComment(void)
{
  WMC_Comment *gfcom = WMC_New(WMC_Comment);
  if (!gfcom)
    return 0;
  gfcom->str = 0;
  gfcom->len = 0;
  gfcom->count = gfcom->cap = 0;
  return gfcom;
}


WMC_Extension *
WMC_NewExtension(int kind, const char* appname, int applength)
{
    WMC_Extension *gfex = WMC_New(WMC_Extension);
    if (!gfex)
        return 0;
    gfex->kind = kind;
    if (appname) {
        gfex->appname = (char*) WMC_NewArray(char, applength + 1);
        if (!gfex->appname) {
            WMC_Delete(gfex);
            return 0;
        }
        memcpy(gfex->appname, appname, applength);
        gfex->appname[applength] = 0;
        gfex->applength = applength;
    } else {
        gfex->appname = 0;
        gfex->applength = 0;
    }
    gfex->data = 0;
    gfex->stream = 0;
    gfex->image = 0;
    gfex->next = 0;
    gfex->free_data = 0;
    gfex->packetized = 0;
    return gfex;
}

WMC_Extension*
WMC_CopyExtension(WMC_Extension* src)
{
    WMC_Extension* dst = WMC_NewExtension(src->kind, src->appname, src->applength);
    if (!dst)
        return NULL;
    if (!src->data || !src->free_data) {
        dst->data = src->data;
        dst->length = src->length;
    } else {
        dst->data = WMC_NewArray(uint8_t, src->length);
        if (!dst->data) {
            WMC_DeleteExtension(dst);
            return NULL;
        }
        memcpy(dst->data, src->data, src->length);
        dst->length = src->length;
        dst->free_data = WMC_Free;
    }
    dst->packetized = src->packetized;
    return dst;
}


char *
WMC_CopyString(const char *s)
{
  int l;
  char *copy;
  if (!s)
    return 0;
  l = (int)strlen(s);
  copy = WMC_NewArray(char, l + 1);
  if (!copy)
    return 0;
  memcpy(copy, s, l + 1);
  return copy;
}


int
WMC_AddImage(WMC_Stream *gfs, WMC_Image *gfi)
{
  if (gfs->nimages >= gfs->imagescap) {
    if (gfs->imagescap)
      gfs->imagescap *= 2;
    else
      gfs->imagescap = 2;
    WMC_ReArray(gfs->images, WMC_Image *, gfs->imagescap);
    if (!gfs->images)
      return 0;
  }
  gfs->images[gfs->nimages] = gfi;
  gfs->nimages++;
  gfi->refcount++;
  return 1;
}


void
WMC_RemoveImage(WMC_Stream *gfs, int inum)
{
  int j;
  if (inum < 0 || inum >= gfs->nimages)
    return;
  WMC_DeleteImage(gfs->images[inum]);
  for (j = inum; j < gfs->nimages - 1; j++)
    gfs->images[j] = gfs->images[j+1];
  gfs->nimages--;
}


int
WMC_ImageColorBound(const WMC_Image* gfi)
{
    if (gfi->compressed)
        return 1 << gfi->compressed[0];
    else
        return 256;
}


int
WMC_AddCommentTake(WMC_Comment *gfcom, char *x, int xlen)
{
  if (gfcom->count >= gfcom->cap) {
    if (gfcom->cap)
      gfcom->cap *= 2;
    else
      gfcom->cap = 2;
    WMC_ReArray(gfcom->str, char *, gfcom->cap);
    WMC_ReArray(gfcom->len, int, gfcom->cap);
    if (!gfcom->str || !gfcom->len)
      return 0;
  }
  if (xlen < 0)
    xlen = (int)strlen(x);
  gfcom->str[ gfcom->count ] = x;
  gfcom->len[ gfcom->count ] = xlen;
  gfcom->count++;
  return 1;
}


int
WMC_AddComment(WMC_Comment *gfcom, const char *x, int xlen)
{
  char *new_x;
  if (xlen < 0)
    xlen = (int)strlen(x);
  new_x = WMC_NewArray(char, xlen);
  if (!new_x)
    return 0;
  memcpy(new_x, x, xlen);
  if (WMC_AddCommentTake(gfcom, new_x, xlen) == 0) {
    WMC_DeleteArray(new_x);
    return 0;
  } else
    return 1;
}


int
WMC_AddExtension(WMC_Stream* gfs, WMC_Image* gfi, WMC_Extension* gfex)
{
    WMC_Extension **pprev;
    if (gfex->stream || gfex->image)
        return 0;
    pprev = gfi ? &gfi->extension_list : &gfs->end_extension_list;
    while (*pprev)
        pprev = &(*pprev)->next;
    *pprev = gfex;
    gfex->stream = gfs;
    gfex->image = gfi;
    gfex->next = 0;
    return 1;
}


int
WMC_ImageNumber(WMC_Stream *gfs, WMC_Image *gfi)
{
    int i;
    if (gfs && gfi)
        for (i = 0; i != gfs->nimages; ++i)
            if (gfs->images[i] == gfi)
                return i;
    return -1;
}


void
WMC_CalculateScreenSize(WMC_Stream *gfs, int force)
{
  int i;
  int screen_width = 0;
  int screen_height = 0;

  for (i = 0; i < gfs->nimages; i++) {
    WMC_Image *gfi = gfs->images[i];
    /* 17.Dec.1999 - I find this old behavior annoying. */
    /* if (gfi->left != 0 || gfi->top != 0) continue; */
    if (screen_width < gfi->left + gfi->width)
      screen_width = gfi->left + gfi->width;
    if (screen_height < gfi->top + gfi->height)
      screen_height = gfi->top + gfi->height;
  }

  /* Only use the default 640x480 screen size if we are being forced to create
     a new screen size or there's no screen size currently. */
  if (screen_width == 0 && (gfs->screen_width == 0 || force))
    screen_width = 640;
  if (screen_height == 0 && (gfs->screen_height == 0 || force))
    screen_height = 480;

  if (gfs->screen_width < screen_width || force)
    gfs->screen_width = screen_width;
  if (gfs->screen_height < screen_height || force)
    gfs->screen_height = screen_height;
}


WMC_Stream *
WMC_CopyStreamSkeleton(WMC_Stream *gfs)
{
  WMC_Stream *ngfs = WMC_NewStream();
  if (!ngfs)
    return 0;
  ngfs->global = WMC_CopyColormap(gfs->global);
  ngfs->background = gfs->background;
  ngfs->screen_width = gfs->screen_width;
  ngfs->screen_height = gfs->screen_height;
  ngfs->loopcount = gfs->loopcount;
  if (gfs->global && !ngfs->global) {
    WMC_DeleteStream(ngfs);
    return 0;
  } else
    return ngfs;
}


WMC_Stream *
WMC_CopyStreamImages(WMC_Stream *gfs)
{
  WMC_Stream *ngfs = WMC_CopyStreamSkeleton(gfs);
  int i;
  if (!ngfs)
    return 0;
  for (i = 0; i < gfs->nimages; i++) {
    WMC_Image *gfi = WMC_CopyImage(gfs->images[i]);
    if (!gfi || !WMC_AddImage(ngfs, gfi)) {
      WMC_DeleteStream(ngfs);
      return 0;
    }
  }
  return ngfs;
}


WMC_Colormap *
WMC_CopyColormap(WMC_Colormap *src)
{
  WMC_Colormap *dest;
  if (!src)
    return 0;

  dest = WMC_NewFullColormap(src->ncol, src->capacity);
  if (!dest)
    return 0;

  memcpy(dest->col, src->col, sizeof(src->col[0]) * src->ncol);
  return dest;
}


WMC_Image *
WMC_CopyImage(WMC_Image *src)
{
  WMC_Image *dest;
  uint8_t *data;
  int i;
  if (!src)
    return 0;

  dest = WMC_NewImage();
  if (!dest)
    return 0;

  dest->identifier = WMC_CopyString(src->identifier);
  if (!dest->identifier && src->identifier)
      goto failure;
  if (src->comment) {
      dest->comment = WMC_NewComment();
      if (!dest->comment)
        goto failure;
      for (i = 0; i < src->comment->count; i++)
        if (!WMC_AddComment(dest->comment, src->comment->str[i],
                            src->comment->len[i]))
          goto failure;
  }
  if (src->extension_list) {
      WMC_Extension* gfex = src->extension_list;
      while (gfex) {
          WMC_Extension* dstex = WMC_CopyExtension(gfex);
          if (!dstex)
              goto failure;
          WMC_AddExtension(NULL, dest, dstex);
          gfex = gfex->next;
      }
  }

  dest->local = WMC_CopyColormap(src->local);
  if (!dest->local && src->local)
    goto failure;
  dest->transparent = src->transparent;

  dest->delay = src->delay;
  dest->disposal = src->disposal;
  dest->left = src->left;
  dest->top = src->top;

  dest->width = src->width;
  dest->height = src->height;

  dest->interlace = src->interlace;
  if (src->img) {
    dest->img = WMC_NewArray(uint8_t *, dest->height + 1);
    dest->image_data = WMC_NewArray(uint8_t, (size_t) dest->width * (size_t) dest->height);
    dest->free_image_data = WMC_Free;
    if (!dest->img || !dest->image_data)
      goto failure;
    for (i = 0, data = dest->image_data; i < dest->height; i++) {
      memcpy(data, src->img[i], dest->width);
      dest->img[i] = data;
      data += dest->width;
    }
    dest->img[dest->height] = 0;
  }
  if (src->compressed) {
    if (src->free_compressed == 0)
      dest->compressed = src->compressed;
    else {
      dest->compressed = WMC_NewArray(uint8_t, src->compressed_len);
      dest->free_compressed = WMC_Free;
      memcpy(dest->compressed, src->compressed, src->compressed_len);
    }
    dest->compressed_len = src->compressed_len;
  }

  return dest;

 failure:
  WMC_DeleteImage(dest);
  return 0;
}


void WMC_MakeImageEmpty(WMC_Image* gfi) {
    WMC_ReleaseUncompressedImage(gfi);
    WMC_ReleaseCompressedImage(gfi);
    gfi->width = gfi->height = 1;
    gfi->transparent = 0;
    WMC_CreateUncompressedImage(gfi, 0);
    gfi->img[0][0] = 0;
}


/** DELETION **/

typedef struct WMC_DeletionHook {
  int kind;
  WMC_DeletionHookFunc func;
  void *callback_data;
  struct WMC_DeletionHook *next;
} WMC_DeletionHook;

static WMC_DeletionHook *all_hooks;

void
WMC_DeleteStream(WMC_Stream *gfs)
{
  WMC_DeletionHook *hook;
  int i;
  if (!gfs || --gfs->refcount > 0)
    return;

  for (i = 0; i < gfs->nimages; i++)
    WMC_DeleteImage(gfs->images[i]);
  WMC_DeleteArray(gfs->images);

  WMC_DeleteColormap(gfs->global);

  WMC_DeleteComment(gfs->end_comment);
  while (gfs->end_extension_list)
      WMC_DeleteExtension(gfs->end_extension_list);

  for (hook = all_hooks; hook; hook = hook->next)
    if (hook->kind == GIF_T_STREAM)
      (*hook->func)(GIF_T_STREAM, gfs, hook->callback_data);
  WMC_Delete(gfs);
}


void
WMC_DeleteImage(WMC_Image *gfi)
{
  WMC_DeletionHook *hook;
  if (!gfi || --gfi->refcount > 0)
    return;

  for (hook = all_hooks; hook; hook = hook->next)
    if (hook->kind == GIF_T_IMAGE)
      (*hook->func)(GIF_T_IMAGE, gfi, hook->callback_data);

  WMC_DeleteArray(gfi->identifier);
  WMC_DeleteComment(gfi->comment);
    while (gfi->extension_list) {
      WMC_DeleteExtension(gfi->extension_list);
        if (gfi->extension_list)
            gfi->extension_list = NULL;
    }
  WMC_DeleteColormap(gfi->local);
  if (gfi->image_data && gfi->free_image_data)
    (*gfi->free_image_data)((void *)gfi->image_data);
  WMC_DeleteArray(gfi->img);
  if (gfi->compressed && gfi->free_compressed)
    (*gfi->free_compressed)((void *)gfi->compressed);
  if (gfi->user_data && gfi->free_user_data)
    (*gfi->free_user_data)(gfi->user_data);
  WMC_Delete(gfi);
}


void
WMC_DeleteColormap(WMC_Colormap *gfcm)
{
  WMC_DeletionHook *hook;
  if (!gfcm || --gfcm->refcount > 0)
    return;

  for (hook = all_hooks; hook; hook = hook->next)
    if (hook->kind == GIF_T_COLORMAP)
      (*hook->func)(GIF_T_COLORMAP, gfcm, hook->callback_data);

  WMC_DeleteArray(gfcm->col);
  WMC_Delete(gfcm);
}


void
WMC_DeleteComment(WMC_Comment *gfcom)
{
  int i;
  if (!gfcom)
    return;
  for (i = 0; i < gfcom->count; i++)
    WMC_DeleteArray(gfcom->str[i]);
  WMC_DeleteArray(gfcom->str);
  WMC_DeleteArray(gfcom->len);
  WMC_Delete(gfcom);
}


void
WMC_DeleteExtension(WMC_Extension *gfex)
{
  if (!gfex)
    return;
  if (gfex->data && gfex->free_data)
    (*gfex->free_data)(gfex->data);
  WMC_DeleteArray(gfex->appname);
  if (gfex->stream || gfex->image) {
      WMC_Extension** pprev;
      if (gfex->image)
          pprev = &gfex->image->extension_list;
      else
          pprev = &gfex->stream->end_extension_list;
      while (*pprev && *pprev != gfex)
          pprev = &(*pprev)->next;
      if (*pprev)
          *pprev = gfex->next;
  }
  WMC_Delete(gfex);
  gfex = NULL;
}


/** DELETION HOOKS **/

int
WMC_AddDeletionHook(int kind, void (*func)(int, void *, void *), void *cb)
{
  WMC_DeletionHook *hook = WMC_New(WMC_DeletionHook);
  if (!hook)
    return 0;
  WMC_RemoveDeletionHook(kind, func, cb);
  hook->kind = kind;
  hook->func = func;
  hook->callback_data = cb;
  hook->next = all_hooks;
  all_hooks = hook;
  return 1;
}

void
WMC_RemoveDeletionHook(int kind, void (*func)(int, void *, void *), void *cb)
{
  WMC_DeletionHook *hook = all_hooks, *prev = 0;
  while (hook) {
    if (hook->kind == kind && hook->func == func
        && hook->callback_data == cb) {
      if (prev)
        prev->next = hook->next;
      else
        all_hooks = hook->next;
      WMC_Delete(hook);
      return;
    }
    prev = hook;
    hook = hook->next;
  }
}


int
WMC_ColorEq(WMC_Color *c1, WMC_Color *c2)
{
  return GIF_COLOREQ(c1, c2);
}


int
WMC_FindColor(WMC_Colormap *gfcm, WMC_Color *c)
{
  int i;
  for (i = 0; i < gfcm->ncol; i++)
    if (GIF_COLOREQ(&gfcm->col[i], c))
      return i;
  return -1;
}


int
WMC_AddColor(WMC_Colormap *gfcm, WMC_Color *c, int look_from)
{
  int i;
  if (look_from >= 0)
    for (i = look_from; i < gfcm->ncol; i++)
      if (GIF_COLOREQ(&gfcm->col[i], c))
        return i;
  if (gfcm->ncol >= gfcm->capacity) {
    gfcm->capacity *= 2;
    WMC_ReArray(gfcm->col, WMC_Color, gfcm->capacity);
    if (gfcm->col == 0)
      return -1;
  }
  i = gfcm->ncol;
  gfcm->ncol++;
  gfcm->col[i] = *c;
  return i;
}


WMC_Image *
WMC_GetImage(WMC_Stream *gfs, int imagenumber)
{
  if (imagenumber >= 0 && imagenumber < gfs->nimages)
    return gfs->images[imagenumber];
  else
    return 0;
}


WMC_Image *
WMC_GetNamedImage(WMC_Stream *gfs, const char *name)
{
  int i;

  if (!name)
    return gfs->nimages ? gfs->images[0] : 0;

  for (i = 0; i < gfs->nimages; i++)
    if (gfs->images[i]->identifier &&
        strcmp(gfs->images[i]->identifier, name) == 0)
      return gfs->images[i];

  return 0;
}


void
WMC_ReleaseCompressedImage(WMC_Image *gfi)
{
  if (gfi->compressed && gfi->free_compressed)
    (*gfi->free_compressed)(gfi->compressed);
  gfi->compressed = 0;
  gfi->compressed_len = 0;
  gfi->free_compressed = 0;
}

void
WMC_ReleaseUncompressedImage(WMC_Image *gfi)
{
  WMC_DeleteArray(gfi->img);
  if (gfi->image_data && gfi->free_image_data)
    (*gfi->free_image_data)(gfi->image_data);
  gfi->img = 0;
  gfi->image_data = 0;
  gfi->free_image_data = 0;
}


//int
//WMC_ClipImage(WMC_Image *gfi, int left, int top, int width, int height)
//{
//  int new_width = gfi->width, new_height = gfi->height;
//  int y;
//
//  if (!gfi->img)
//    return 0;
//
//  if (gfi->left < left) {
//    int shift = left - gfi->left;
//    for (y = 0; y < gfi->height; y++)
//      gfi->img[y] += shift;
//    gfi->left += shift;
//    new_width -= shift;
//  }
//
//  if (gfi->top < top) {
//    int shift = top - gfi->top;
//    for (y = gfi->height - 1; y >= shift; y++)// 这个for循环有问题
//      gfi->img[y - shift] = gfi->img[y];
//    gfi->top += shift;
//    new_height -= shift;
//  }
//
//  if (gfi->left + new_width >= width)
//    new_width = width - gfi->left;
//
//  if (gfi->top + new_height >= height)
//    new_height = height - gfi->top;
//
//  if (new_width < 0)
//    new_width = 0;
//  if (new_height < 0)
//    new_height = 0;
//  gfi->width = new_width;
//  gfi->height = new_height;
//  return 1;
//}


int
WMC_InterlaceLine(int line, int height)
{
  height--;
  if (line > height / 2)
    return line * 2 - ( height       | 1);
  else if (line > height / 4)
    return line * 4 - ((height & ~1) | 2);
  else if (line > height / 8)
    return line * 8 - ((height & ~3) | 4);
  else
    return line * 8;
}


int
WMC_SetUncompressedImage(WMC_Image *gfi, uint8_t *image_data,
                         void (*free_data)(void *), int data_interlaced)
{
  /* NB does not affect compressed image (and must not) */
  unsigned i;
  unsigned width = gfi->width;
  unsigned height = gfi->height;
  uint8_t **img;

  WMC_ReleaseUncompressedImage(gfi);
  if (!image_data)
    return 0;

  img = WMC_NewArray(uint8_t *, height + 1);
  if (!img)
    return 0;

  if (data_interlaced)
    for (i = 0; i < height; i++)
      img[ WMC_InterlaceLine(i, height) ] = image_data + width * i;
  else
    for (i = 0; i < height; i++)
      img[i] = image_data + width * i;
  img[height] = 0;

  gfi->img = img;
  gfi->image_data = image_data;
  gfi->free_image_data = free_data;
  return 1;
}

int
WMC_CreateUncompressedImage(WMC_Image *gfi, int data_interlaced)
{
    size_t sz = (size_t) gfi->width * (size_t) gfi->height;
    uint8_t *data = WMC_NewArray(uint8_t, sz ? sz : 1);
    return WMC_SetUncompressedImage(gfi, data, WMC_Free, data_interlaced);
}

void
WMC_InitCompressInfo(WMC_CompressInfo *gcinfo)
{
    gcinfo->flags = 0;
    gcinfo->loss = 0;
}


void
WMC_Debug(char *x, ...)
{
    va_list val;
    va_start(val, x);
    vfprintf(stderr, x, val);
    va_end(val);
}


#if !GIF_ALLOCATOR_DEFINED
void* WMC_Realloc(void* p, size_t s, size_t n, const char* file, int line) {
    (void) file, (void) line;
    if (s == 0 || n == 0)
        WMC_Free(p);
    else if (s == 1 || n == 1 || s <= ((size_t) -1) / n)
        return realloc(p, s * n);
    return (void*) 0;
}

#undef WMC_Free
void WMC_Free(void* p) {
    free(p);
}
#endif

#ifdef __cplusplus
}
#endif
