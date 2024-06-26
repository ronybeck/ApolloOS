/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$
*/

#ifndef GFX_HIDD_INTERN_H
#define GFX_HIDD_INTERN_H

/* Include files */

#include <aros/debug.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <oop/oop.h>
#include <hidd/gfx.h>
#include <dos/dos.h>
#include <graphics/gfxbase.h>
#include <graphics/monitor.h>

#define USE_FAST_GETPIXEL		1
#define USE_FAST_PUTPIXEL		1
#define OPTIMIZE_DRAWPIXEL_FOR_COPY	1
#define USE_FAST_DRAWPIXEL		1
#define USE_FAST_DRAWLINE               1
#define USE_FAST_GETIMAGE               1
#define USE_FAST_PUTIMAGE               1
#define USE_FAST_CONVERTPIXELS          1
#define USE_FAST_UNMAPPIXEL             1
#define USE_FAST_MAPCOLOR               1
#define COPYBOX_CHECK_FOR_ALIKE_PIXFMT	1

#define USE_FAST_BMHGETPIXEL            1
#define USE_FAST_BMHUNMAPPIXEL          1
#define USE_FAST_BMHFINDCOLOR           1
#define USE_FAST_BMHADDCOLOR            1

struct HWGfxData
{
};

#define HBM(x) ((struct HIDDBitMapData *)x)
#define HBMH(x) ((struct HiddBMHistogramData *)x)

#define GOT_PF_ATTR(code)	GOT_ATTR(code, aoHidd_PixFmt, pixfmt)
#define FOUND_PF_ATTR(code)	FOUND_ATTR(code, aoHidd_PixFmt, pixfmt);

#define GOT_SYNC_ATTR(code)	GOT_ATTR(code, aoHidd_Sync, sync)
#define FOUND_SYNC_ATTR(code)	FOUND_ATTR(code, aoHidd_Sync, sync);

#define GOT_BM_ATTR(code)	GOT_ATTR(code, aoHidd_BitMap, bitmap)
#define FOUND_BM_ATTR(code)	FOUND_ATTR(code, aoHidd_BitMap, bitmap);

#define SWAPBYTES_WORD(x) ((((x) >> 8) & 0x00FF) | (((x) & 0x00FF) << 8))

struct colormap_data
{
    HIDDT_ColorLUT clut;
};

struct pixfmt_data
{
     HIDDT_PixelFormat pf; /* Public portion in the beginning    */

     struct MinNode node;  /* Node for linking into the database */
     ULONG refcount;	   /* Reference count			 */
};

/* Use this macro in order to transform node pointer to pixfmt pointer */
#define PIXFMT_OBJ(n) ((HIDDT_PixelFormat *)((char *)(n) - offsetof(struct pixfmt_data, node)))

struct planarbm_data
{
    struct BitMap *bitmap;		/* Associated BitMap structure		  */
    BOOL	   planes_alloced;	/* Whether the BitMap was allocated by us */
};

struct chunkybm_data
{
    OOP_Object *gfxhidd;       	/* Cached driver object				*/
    UBYTE      *buffer;		/* Pixelbuffer		  			*/
    ULONG	bytesperrow;	/* Cached for faster access 			*/
    UWORD	bytesperpixel;
    BOOL	own_buffer;	/* Whether the buffer was allocated by us	*/
};

struct sync_data
{
    struct MonitorSpec *mspc;	/* Associated MonitorSpec */

    ULONG pixelclock;		/* pixel time in Hz */

    ULONG hdispmax;
    ULONG vdispmax;
    ULONG hdisp;		/* Data missing from MonitorSpec */
    ULONG htotal;
    ULONG vdisp;

    ULONG flags;		/* Flags */

    UBYTE description[32];
    
    ULONG hmin;			/* Minimum and maximum allowed bitmap size */
    ULONG hmax;
    ULONG vmin;
    ULONG vmax;

    OOP_Object	 *gfxhidd;	 /* Graphics driver that owns this sync		*/
    ULONG InternalFlags;	 /* Internal flags, see below			*/
};

/* Sync internal flags */
#define SYNC_FREE_MONITORSPEC    0x0001 /* Allocated own MonitorSpec 				*/
#define SYNC_FREE_SPECIALMONITOR 0x0002 /* Allocated own SpecialMonitor				*/
#define SYNC_VARIABLE		 0x0004 /* Signal timings can be changed			*/

struct gc_data
{
    HIDDT_GC_Intern  prot;
    struct Rectangle cr;
};

struct mode_bm {
    UBYTE *bm;
    UWORD bpr;
};
struct mode_db {
    /* Array of all available gfxmode PixFmts that are part of 
       gfxmodes
     */
    struct SignalSemaphore sema;
    OOP_Object **pixfmts;
    /* Number of pixfmts in the above array */
    ULONG num_pixfmts;
    
    /* All the sync times that are part of any gfxmode */
    OOP_Object **syncs;
    /* Number of syncs in the above array */
    ULONG num_syncs;
    
    /* A bitmap of size (num_pixfmts * num_syncs), that tells if the
       mode is displayable or not. If a particular (x, y) coordinate
       of the bitmap is 1, it means that the pixfmt and sync objects
       you get by indexing pixfmts[x] and syncs[y] are a valid mode.
       If not, the mode is considered invalid
    */
    
    struct mode_bm orig_mode_bm;	/* Original as supplied by subclass */
    struct mode_bm checked_mode_bm;	/* After applying monitor refresh rate checks etc. */
    
};

struct HiddGfxData
{
	/* Gfx mode "database" */
	struct mode_db mdb;

	/* Framebuffer control stuff */
	OOP_Object *framebuffer;
	OOP_Object *shownbm;
	BYTE        fbmode;
	struct SignalSemaphore fbsem;

	/* gc used for stuff like rendering cursor */
	OOP_Object *gc;
};

/* Private gfxhidd methods */
OOP_Object *GFXHIDD__Hidd_Gfx__RegisterPixFmt(OOP_Class *cl, struct TagItem *pixFmtTags);
VOID GFXHIDD__Hidd_Gfx__ReleasePixFmt(OOP_Class *cl, OOP_Object *pf);

static inline BOOL GFXHIDD__Hidd_Gfx__SetFBColors(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_SetColors *msg)
{
    struct HiddGfxData *data = OOP_INST_DATA(cl, o);

    return OOP_DoMethod(data->framebuffer, &msg->mID);
}    

static inline UBYTE GFXHIDD__Hidd_Gfx__GetFBModeQuick(OOP_Class *cl, OOP_Object *o)
{
    struct HiddGfxData *data = OOP_INST_DATA(cl, o);

    return data->fbmode;
}

/* This has to be a #define, otherwise the HIDD_Gfx_CopyBox()
 * macro won't expand the HiddGfxBase macro to the csd
 * structure.
 */
#define GFXHIDD__Hidd_Gfx__UpdateFB(cl, o, bm, srcX, srcY, destX, destY, xSize, ySize) \
do { \
    struct HiddGfxData *__data = OOP_INST_DATA(cl, o); \
    HIDD_Gfx_CopyBox(o, bm, srcX, srcY, \
                     __data->framebuffer, destX, destY, \
                     xSize, ySize, __data->gc); \
} while (0)

/* Private bitmap methods */
void BM__Hidd_BitMap__SetBitMapTags(OOP_Class *cl, OOP_Object *o, struct TagItem *bitMapTags);
void BM__Hidd_BitMap__SetPixFmt(OOP_Class *cl, OOP_Object *o, OOP_Object *pf);
void BM__Hidd_BitMap__SetVisible(OOP_Class *cl, OOP_Object *o, BOOL val);

struct HIDDBitMapData
{
    struct _hidd_bitmap_protected prot;

    struct BitMap          *bmstruct;
    UWORD                  width;         /* width of the bitmap in pixel                  */
    UWORD                  height;        /* height of the bitmap in pixel                 */
    UWORD                  align;         /* Default alignment                             */
    IPTR                   displayWidth;  /* Display size                                  */
    IPTR                   displayHeight;
    struct Rectangle       display;       /* Display rectangle (in bitmap's coordinates !) */
    struct SignalSemaphore lock;          /* Scroll/update semaphore                       */
    BOOL                   visible;       /* bitmap visible ?                              */
    BOOL                   displayable;   /* bitmap directly displayable?                  */
    BOOL                   compositable;  /* bitmap displayable via compositor             */
    BOOL                   framebuffer;	  /* is a framebuffer ?		                   */
    BOOL                   pf_registered; /* Registered own pixelformat ?	           */
    ULONG                  flags;         /* see hidd/graphic.h 'flags for                 */
    ULONG                  bytesPerRow;   /* bytes per row                                 */
    OOP_Object            *friend;        /* Friend bitmap		                   */
    OOP_Object            *gfxhidd;       /* Owning driver		                   */
    OOP_Object            *colmap;        /* Colormap                                      */
    OOP_Object            *gc;            /* Shared GC for copy operations                 */
    HIDDT_ModeID           modeid;        /* Display mode ID		                   */

    /* Optimize these method calls */
#if USE_FAST_PUTPIXEL
    OOP_MethodFunc         putpixel;
    OOP_Class 	          *putpixel_Class;
#endif
#if USE_FAST_GETPIXEL
    OOP_MethodFunc         getpixel;
    OOP_Class	          *getpixel_Class;
#endif
#if USE_FAST_DRAWPIXEL
    OOP_MethodFunc         drawpixel;
    OOP_Class             *drawpixel_Class;
#endif
#if USE_FAST_DRAWLINE
    OOP_MethodFunc         drawline;
    OOP_Class             *drawline_Class;
#endif
#if USE_FAST_GETIMAGE
    OOP_MethodFunc         getimage;
    OOP_Class             *getimage_Class;
#endif
#if USE_FAST_PUTIMAGE
    OOP_MethodFunc         putimage;
    OOP_Class             *putimage_Class;
#endif
#if USE_FAST_CONVERTPIXELS
    OOP_MethodFunc         convertpixels;
    OOP_Class             *convertpixels_Class;
#endif
#if USE_FAST_UNMAPPIXEL
    OOP_MethodFunc         unmappixel;
    OOP_Class             *unmappixel_Class;
#endif
#if USE_FAST_MAPCOLOR
    OOP_MethodFunc         mapcolor;
    OOP_Class             *mapcolor_Class;
#endif
};

struct BMHistEntry
{
    struct BMHistEntry  *next;
    HIDDT_Color         color;
    ULONG               count;
};

struct HiddBMHistogramData
{
    struct SignalSemaphore sema;
    APTR                hpool;
    ULONG               size;
    struct BMHistEntry  *hist;

    /* Optimize these method calls */
#if USE_FAST_BMHGETPIXEL
    OOP_MethodFunc         getpixel;
    OOP_Class 	          *getpixel_Class;
#endif
#if USE_FAST_BMHUNMAPPIXEL
    OOP_MethodFunc         unmappixel;
    OOP_Class	          *unmappixel_Class;
#endif
#if USE_FAST_BMHFINDCOLOR
    OOP_MethodFunc         findcolorentry;
    OOP_Class             *findcolorentry_Class;
#endif
#if USE_FAST_BMHADDCOLOR
    OOP_MethodFunc         addcolorentry;
    OOP_Class             *addcolorentry_Class;
#endif
};

#define NUM_ATTRBASES   12
#define NUM_METHODBASES 6

struct class_static_data
{
    struct GfxBase	 *cs_GfxBase;
    struct Library	 *cs_UtilityBase;
    struct Library	 *cs_OOPBase;
    BPTR                  cs_SegList;

    struct SignalSemaphore sema;

    OOP_AttrBase	 attrBases[NUM_ATTRBASES];
    OOP_MethodID	 methodBases[NUM_METHODBASES];

    OOP_Class            *gfxhwclass;                   /* graphics hw enumerator class         */
    OOP_Class            *gfxhiddclass; /* graphics hidd class    */
    OOP_Class            *bitmapclass;  /* bitmap class           */
    OOP_Class            *bmhistogramclass;
    OOP_Class            *gcclass;      /* graphics context class */
    OOP_Class		 *colormapclass; /* colormap class	  */
    
    OOP_Class		 *pixfmtclass;	/* describing bitmap pixel formats */
    OOP_Class		 *syncclass;	/* describing gfxmode sync times */
    
    OOP_Class		 *planarbmclass;
    OOP_Class		 *chunkybmclass;

    OOP_Object           *gfxhwinstance;

    /*
       Pixel format "database". This is a list
       of all pixelformats currently used by some bitmap.
       The point of having this as a central db in the gfx hidd is
       that if several bitmaps are of the same pixel format
       they may point to the same PixFmt object instead
       of allocating their own instance. Thus we are saving mem
     */
    struct SignalSemaphore pfsema;
    struct MinList pflist;
    /* Index of standard pixelformats for quick access */
    HIDDT_PixelFormat	 *std_pixfmts[num_Hidd_StdPixFmt];

    HIDDT_RGBConversionFunction rgbconvertfuncs[NUM_RGB_STDPIXFMT][NUM_RGB_STDPIXFMT];
    struct SignalSemaphore rgbconvertfuncs_sem;
};

#define __IHidd_BitMap	    (csd->attrBases[0])
#define __IHidd_BMHistogram (csd->attrBases[1])
#define __IHidd_Gfx 	    (csd->attrBases[2])
#define __IHidd_GC  	    (csd->attrBases[3])
#define __IHidd_ColorMap    (csd->attrBases[4])
#define __IHW 	            (csd->attrBases[5])
#define __IHidd             (csd->attrBases[6])
#define __IHidd_Overlay	    (csd->attrBases[7])
#define __IHidd_Sync	    (csd->attrBases[8])
#define __IHidd_PixFmt      (csd->attrBases[9])
#define __IHidd_PlanarBM    (csd->attrBases[10])
#define __IHidd_ChunkyBM    (csd->attrBases[11])

#undef HiddBitMapBase
#undef HiddBMHistogramBase
#undef HiddGfxBase
#undef HiddGCBase
#undef HiddColorMapBase
#undef HWBase
#define HiddBitMapBase	    (csd->methodBases[0])
#define HiddBMHistogramBase (csd->methodBases[1])
#define HiddGfxBase	    (csd->methodBases[2])
#define HiddGCBase	    (csd->methodBases[3])
#define HiddColorMapBase    (csd->methodBases[4])
#define HWBase              (csd->methodBases[5])

/* Library base */

struct IntHIDDGraphicsBase
{
    struct Library            hdg_LibNode;

    struct class_static_data  hdg_csd;
};


/* pre declarations */

BOOL parse_pixfmt_tags(struct TagItem *tags, HIDDT_PixelFormat *pf, ULONG attrcheck, struct class_static_data *csd);

static inline ULONG color_distance(UWORD a1, UWORD r1, UWORD g1, UWORD b1, UWORD a2, UWORD r2, UWORD g2, UWORD b2)
{
    /* NOTE: The use of 'WORD' here and the 'UWORD' casts below are
     *       important hints to GCC to generate better code on m68k
     */
    LONG da = (a1 >> 8) - (a2 >> 8);
    LONG dr = (r1 >> 8) - (r2 >> 8);
    LONG dg = (g1 >> 8) - (g2 >> 8);
    LONG db = (b1 >> 8) - (b2 >> 8);

    DB2(bug("[color_distance] a1 = 0x%04X a2 = 0x%04X da = %d\n", a1, a2, da));
    DB2(bug("[color_distance] r1 = 0x%04X r2 = 0x%04X dr = %d\n", r1, r2, dr));
    DB2(bug("[color_distance] g1 = 0x%04X g2 = 0x%04X dg = %d\n", g1, g2, dg));
    DB2(bug("[color_distance] b1 = 0x%04X b2 = 0x%04X db = %d\n", b1, b2, db));

    /* '4' here is a result of trial and error. The idea behind this is to increase
       the weight of alpha difference in order to make the function prefer colors with
       the same alpha value. This is important for correct mouse pointer remapping. */
    return (ULONG)(dr*dr) + (ULONG)(dg*dg) + (ULONG)(db*db);
}

#define CSD(x) (&((struct IntHIDDGraphicsBase *)x->UserData)->hdg_csd)
#define csd CSD(cl)

/* The following calls are optimized by calling the method functions directly */

#if USE_FAST_GETPIXEL
static inline HIDDT_Pixel GETPIXEL(OOP_Class *cl, OOP_Object *o, WORD x, WORD y)
{
    struct pHidd_BitMap_GetPixel getp_p;

    getp_p.mID = HiddBitMapBase + moHidd_BitMap_GetPixel;
    getp_p.x   = x;
    getp_p.y   = y;

    return HBM(o)->getpixel(HBM(o)->getpixel_Class, o, &getp_p.mID);
}
#else
#define GETPIXEL(cl, obj, x, y) HIDD_BM_GetPixel(obj, x, y)
#endif

#if USE_FAST_PUTPIXEL
static inline void PUTPIXEL(OOP_Class *cl, OOP_Object *o, WORD x, WORD y, HIDDT_Pixel val)
{
    struct pHidd_BitMap_PutPixel putp_p;

    putp_p.mID   = HiddBitMapBase + moHidd_BitMap_PutPixel;
    putp_p.x     = x;
    putp_p.y     = y;
    putp_p.pixel = val;

    HBM(o)->putpixel(HBM(o)->putpixel_Class, o, &putp_p.mID);
}
#else
#define PUTPIXEL(cl, obj, x, y, val) HIDD_BM_PutPixel(obj, x, y, val)
#endif

#if USE_FAST_DRAWPIXEL
static inline void DRAWPIXEL(OOP_Class *cl, OOP_Object *o, OOP_Object *gc, WORD x, WORD y)
{
    struct pHidd_BitMap_DrawPixel drawp_p;

    drawp_p.mID = HiddBitMapBase + moHidd_BitMap_DrawPixel;
    drawp_p.gc  = gc;
    drawp_p.x   = x;
    drawp_p.y   = y;

    HBM(o)->drawpixel(HBM(o)->drawpixel_Class, o, &drawp_p.mID);
}
#else
#define DRAWPIXEL(cl, obj, gc, x, y) HIDD_BM_PutPixel(obj, gc, x, y)
#endif

#if USE_FAST_DRAWLINE
static inline void DRAWLINE(OOP_Class *cl, OOP_Object *o, OOP_Object *gc, WORD x1, WORD y1, WORD x2, WORD y2)
{
    struct pHidd_BitMap_DrawLine drawl_p;

    drawl_p.mID = HiddBitMapBase + moHidd_BitMap_DrawLine;
    drawl_p.gc   = gc;
    drawl_p.x1   = x1;
    drawl_p.y1   = y1;
    drawl_p.x2   = x2;
    drawl_p.y2   = y2;

    HBM(o)->drawline(HBM(o)->drawline_Class, o, &drawl_p.mID);
}
#else
#define DRAWLINE(cl, obj, gc, x1, y1, x2, y2) HIDD_BM_DrawLine(obj, gc, x1, y1, x2, y2)
#endif

#if USE_FAST_GETIMAGE
static inline void GETIMAGE(OOP_Class *cl, OOP_Object *o, UBYTE *pixels, ULONG modulo, WORD x, WORD y,
                               WORD width, WORD height, HIDDT_StdPixFmt pixFmt)
{
    struct pHidd_BitMap_GetImage geti_p;

    geti_p.mID = HiddBitMapBase + moHidd_BitMap_GetImage;
    geti_p.pixels       = pixels;
    geti_p.modulo       = modulo;
    geti_p.x            = x;
    geti_p.y            = y;
    geti_p.width        = width;
    geti_p.height       = height;
    geti_p.pixFmt       = pixFmt;

    HBM(o)->getimage(HBM(o)->getimage_Class, o, &geti_p.mID);
}
#else
#define GETIMAGE(cl, o, obj, pixels, modulo, x, y, width, height, pixFmt) HIDD_BM_GetImage(o, obj, pixels, modulo, x, y, width, height, pixFmt)
#endif

#if USE_FAST_PUTIMAGE
static inline void PUTIMAGE(OOP_Class *cl, OOP_Object *o, OOP_Object *gc, UBYTE *pixels, ULONG modulo,
                               WORD x, WORD y, WORD width, WORD height, HIDDT_StdPixFmt pixFmt)
{
    struct pHidd_BitMap_PutImage puti_p;

    puti_p.mID = HiddBitMapBase + moHidd_BitMap_PutImage;
    puti_p.gc           = gc;
    puti_p.pixels       = pixels;
    puti_p.modulo       = modulo;
    puti_p.x            = x;
    puti_p.y            = y;
    puti_p.width        = width;
    puti_p.height       = height;
    puti_p.pixFmt       = pixFmt;

    HBM(o)->putimage(HBM(o)->putimage_Class, o, &puti_p.mID);
}
#else
#define PUTIMAGE(cl, obj, gc, pixels, modulo, x, y, width, height, pixFmt) HIDD_BM_PutImage(obj, gc, pixels, modulo, x, y, width, height, pixFmt)
#endif

#if USE_FAST_CONVERTPIXELS
static inline void CONVERTPIXELS(OOP_Class *cl, OOP_Object *o, APTR *srcPixels, HIDDT_PixelFormat *srcPixFmt, ULONG srcMod, APTR *dstBuf, HIDDT_PixelFormat *dstPixFmt, ULONG dstMod, UWORD width, UWORD height, HIDDT_PixelLUT *pixlut)
{
    struct pHidd_BitMap_ConvertPixels convp_p;

    convp_p.mID = HiddBitMapBase + moHidd_BitMap_ConvertPixels;
    convp_p.srcPixels   = srcPixels;
    convp_p.srcPixFmt   = srcPixFmt;
    convp_p.srcMod      = srcMod;
    convp_p.dstBuf      = dstBuf;
    convp_p.dstPixFmt   = dstPixFmt;
    convp_p.dstMod      = dstMod;
    convp_p.width       = width;
    convp_p.height      = height;
    convp_p.pixlut      = pixlut;

    HBM(o)->convertpixels(HBM(o)->convertpixels_Class, o, &convp_p.mID);
}
#else
#define CONVERTPIXELS(cl, obj, srcPixels, srcPixFmt, srcMod, dstBuf, dstPixFmt, dstMod, width, height, pixlut) HIDD_BM_ConvertPixels(obj, srcPixels, srcPixFmt, srcMod, dstBuf, dstPixFmt, dstMod, width, height, pixlut)
#endif

#if USE_FAST_UNMAPPIXEL
static inline void UNMAPPIXEL(OOP_Class *cl, OOP_Object *o, HIDDT_Pixel pixel, HIDDT_Color *color)
{
    struct pHidd_BitMap_UnmapPixel unmpp_p;

    unmpp_p.mID = HiddBitMapBase + moHidd_BitMap_UnmapPixel;
    unmpp_p.pixel   = pixel;
    unmpp_p.color   = color;

    HBM(o)->unmappixel(HBM(o)->unmappixel_Class, o, &unmpp_p.mID);
}
#else
#define UNMAPPIXEL(cl, obj, pixel, color) HIDD_BM_UnmapPixel(obj, pixel, color)
#endif

//HIDD_BM_MapColor(o, &col)
#if USE_FAST_MAPCOLOR
static inline HIDDT_Pixel MAPCOLOR(OOP_Class *cl, OOP_Object *o, HIDDT_Color *color)
{
    struct pHidd_BitMap_MapColor mapc_p;

    mapc_p.mID = HiddBitMapBase + moHidd_BitMap_MapColor;
    mapc_p.color   = color;

    return HBM(o)->mapcolor(HBM(o)->mapcolor_Class, o, &mapc_p.mID);
}
#else
#define MAPCOLOR(cl, obj, color) HIDD_BM_MapColor(obj, color)
#endif

/* BMHistograms operations ... */
#if USE_FAST_BMHGETPIXEL
static inline HIDDT_Pixel BMHGETPIXEL(OOP_Class *cl, OOP_Object *o, WORD x, WORD y)
{
    struct pHidd_BitMap_GetPixel getp_p;

    getp_p.mID          = HiddBitMapBase + moHidd_BitMap_GetPixel;
    getp_p.x            = x;
    getp_p.y            = y;

    return (HIDDT_Pixel)HBM(o)->getpixel(HBM(o)->getpixel_Class, o, &getp_p.mID);
}
#else
#define BMHGETPIXEL(cl, obj, x, y) HIDD_BM_GetPixel(obj, x, y)
#endif

#if USE_FAST_BMHUNMAPPIXEL
static inline void BMHUNMAPPIXEL(OOP_Class *cl, OOP_Object *o, HIDDT_Pixel pixel, HIDDT_Color *color)
{
    struct pHidd_BitMap_UnmapPixel ump_p;

    ump_p.mID           = HiddBitMapBase + moHidd_BitMap_UnmapPixel;
    ump_p.pixel         = pixel;
    ump_p.color         = color;

    HBM(o)->unmappixel(HBM(o)->unmappixel_Class, o, &ump_p.mID);
}
#else
#define BMHUNMAPPIXEL(cl, obj, pixel, color) HIDD_BM_UnmapPixel(obj, pixel, color)
#endif

#if USE_FAST_BMHFINDCOLOR
static inline IPTR BMHFINDCOLOR(OOP_Class *cl, OOP_Object *o, HIDDT_Color *color, APTR *colentry)
{
    struct pHidd_BMHistogram_FindColorEntry fce_p;

    fce_p.mID           = HiddBMHistogramBase + moHidd_BMHistogram_FindColorEntry;
    fce_p.color         = color;
    fce_p.colentry      = colentry;

    return (IPTR)HBMH(o)->findcolorentry(HBMH(o)->findcolorentry_Class, o, &fce_p.mID);
}
#else
#define BMHFINDCOLOR(cl, obj, color, colentry) HIDD_BMHISTOGRAM_FindColorEntry(obj, color, colentry)
#endif

#if USE_FAST_BMHADDCOLOR
static inline APTR BMHADDCOLOR(OOP_Class *cl, OOP_Object *o, HIDDT_Color *color, ULONG count)
{
    struct pHidd_BMHistogram_AddColorEntry ace_p;

    ace_p.mID           = HiddBMHistogramBase + moHidd_BMHistogram_AddColorEntry;
    ace_p.color         = color;
    ace_p.count         = count;

    return (APTR)HBMH(o)->addcolorentry(HBMH(o)->addcolorentry_Class, o, &ace_p.mID);
}
#else
#define BMHADDCOLOR(cl, obj, color, count) HIDD_BMHISTOGRAM_AddColorEntry(obj, color, count)
#endif

#endif /* GFX_HIDD_INTERN_H */
