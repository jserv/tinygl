/*
 * Template-based rasterizer variant generation.
 *
 * This header generates specialized triangle rasterization functions at
 * compile-time for different combinations of:
 *   - depth_test (DT): 0 = disabled, 1 = enabled
 *   - depth_write (DW): 0 = disabled, 1 = enabled
 *
 * This eliminates per-pixel runtime branching for these state flags, improving
 * performance in hot rasterization loops.
 *
 * Each base function (Flat, Smooth, Textured) gets 4 variants:
 *   - DT0_DW0: always pass depth, never write depth
 *   - DT0_DW1: always pass depth, always write depth
 *   - DT1_DW0: test depth, never write depth
 *   - DT1_DW1: test depth, always write depth
 */

#ifndef ZTRIANGLE_VARIANTS_H
#define ZTRIANGLE_VARIANTS_H

/*
 * Compile-time depth test macro.
 * When ZTRI_DEPTH_TEST is 0, depth test always passes.
 * When ZTRI_DEPTH_TEST is 1, actual depth comparison is performed.
 */
#ifdef ZTRI_DEPTH_TEST
#if ZTRI_DEPTH_TEST == 0
#define ZCMP_VARIANT(z, zpix) (1)
#define ZCMPSIMP_VARIANT(z, zpix, _a) (1 STIPTEST(_a))
#define ZCMP_FULL_VARIANT(z, zpix, _a, c) (1 STIPTEST(_a) NODRAWTEST(c))
#else
#define ZCMP_VARIANT(z, zpix) ((z) >= (zpix))
#define ZCMPSIMP_VARIANT(z, zpix, _a) (((z) >= (zpix)) STIPTEST(_a))
#define ZCMP_FULL_VARIANT(z, zpix, _a, c) \
    (((z) >= (zpix)) STIPTEST(_a) NODRAWTEST(c))
#endif
#endif

/*
 * Compile-time depth write macro.
 * When ZTRI_DEPTH_WRITE is 0, depth buffer is never written.
 * When ZTRI_DEPTH_WRITE is 1, depth buffer is always written on pass.
 */
#ifdef ZTRI_DEPTH_WRITE
#if ZTRI_DEPTH_WRITE == 0
#define DEPTH_WRITE_VARIANT(pz, _a, zz) /* no-op */
#else
#define DEPTH_WRITE_VARIANT(pz, _a, zz) pz[_a] = zz
#endif
#endif

/*
 * Function naming macros for variant generation.
 * Creates names like: ZB_fillTriangleFlat_DT0_DW1
 */
#define VARIANT_SUFFIX_EXPAND(dt, dw) _DT##dt##_DW##dw
#define VARIANT_SUFFIX(dt, dw) VARIANT_SUFFIX_EXPAND(dt, dw)

#define VARIANT_NAME_EXPAND(base, suffix) base##suffix
#define VARIANT_NAME(base, suffix) VARIANT_NAME_EXPAND(base, suffix)

/*
 * Dispatch table index calculation.
 * Index = (depth_test << 1) | depth_write
 *   0 = DT0_DW0
 *   1 = DT0_DW1
 *   2 = DT1_DW0
 *   3 = DT1_DW1
 */
#define ZTRI_VARIANT_INDEX(dt, dw) (((dt) << 1) | (dw))
#define ZTRI_VARIANT_COUNT 4

/*
 * ZB_fillTriangleFunc is defined in zbuffer.h
 * We use it for the dispatch table below.
 */

/*
 * Dispatch table structure for each shading mode.
 */
typedef struct {
    ZB_fillTriangleFunc flat[ZTRI_VARIANT_COUNT];
    ZB_fillTriangleFunc flat_noblend[ZTRI_VARIANT_COUNT];
    ZB_fillTriangleFunc smooth[ZTRI_VARIANT_COUNT];
    ZB_fillTriangleFunc smooth_noblend[ZTRI_VARIANT_COUNT];
    ZB_fillTriangleFunc textured[ZTRI_VARIANT_COUNT];
    ZB_fillTriangleFunc textured_noblend[ZTRI_VARIANT_COUNT];
} ZB_TriangleDispatch;

/*
 * Global dispatch table (defined in ztriangle.c)
 */
extern ZB_TriangleDispatch zb_triangle_dispatch;

/*
 * Inline dispatch function for selecting the right variant.
 */
static inline ZB_fillTriangleFunc ZB_getTriangleFunc(
    const ZB_fillTriangleFunc *table,
    GLint depth_test,
    GLint depth_write)
{
    return table[ZTRI_VARIANT_INDEX(depth_test != 0, depth_write != 0)];
}

#endif /* ZTRIANGLE_VARIANTS_H */
