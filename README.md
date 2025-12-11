# TinyGL Evolved

![tgl logo](assets/tgl_minimal.png)

A portable, software-only OpenGL 1.1 rasterizer written in pure C99.

Originally developed by [Fabrice Bellard](https://bellard.org/), this version has been extensively enhanced for performance and portability.
It is a small, partial OpenGL 1.1 implementation with optional multithreading support.

## Architecture
TinyGL consists of three major modules:
- Mathematical routines (zmath)
- OpenGL-like emulation (zgl)
- Z-buffer and rasterization (zbuffer)

## Safety Features

TinyGL includes the following safety features:
1. Compile-time options for `glGetError()` functionality (useful for debugging)
2. OpenGL 2.0 style buffers for simplified memory management (data passed via `glBufferData` is freed upon `glClose()`)
3. Fully leak-checked using Valgrind (the raw demos have zero leaks)

## Portability

TinyGL is written in pure C99 with minimal standard library dependencies.
It does not require `malloc` or `free` directly; allocation calls are aliased to `gl_malloc()` and `gl_free()`,
so you can drop in a custom allocator.

Sanity-check portability from the repo root with `make raw_examples`, which builds the raw demos using only the C standard library.

The raw examples use these standard library headers:
```c
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
```

If your system supports it, the library can use `alignas` for improved SIMD support. This adds a dependency on `<stdalign.h>` and can increase vertex processing speed (disabled by default for maximum portability).

If you are unsure whether your target platform can support TinyGL, compile with the build-time and runtime tests enabled (they are enabled by default):
- A `TGL_BUILDT` error indicates a failed build-time test
- `TINYGL_FAILED_RUNTIME_COMPAT_TEST` printed to stdout indicates a failed runtime test

The SDL examples have been tested on Debian 10 and Windows 10, while the library itself has been confirmed to compile on many more platforms.

## Notable Changes

Compared to the original TinyGL:

Rendering:
* Disabled 8, 15, and 24-bit rendering modes; only 16-bit and 32-bit modes are supported (also the fastest)
* Blending support
* Triangles can be lit and textured simultaneously
* Line rendering obeys `glDepthMask` and `glDepthTest`
* Polygon stipple support
* Fixed specular rendering
* Tuned triangle rasterizer and transformation pipeline

Performance Optimizations:
* Dirty rectangle tracking for partial framebuffer updates
* Template-based rasterizer for reduced branching
* Newton-Raphson approximation for inner loop division
* Optimized depth buffer clear
* Eliminated redundant context lookups in hot paths
* OpenMP multithreading for `glDrawPixels`, `glPostProcess()`, `glCopyTexImage2D`, and `ZB_copyBuffer`

API Additions:
* `glDrawPixels`, `glPixelZoom`
* `glRasterPos2f/3f/4f/2fv/3fv/4fv`
* `glGetString()` for `GL_VENDOR`, `GL_RENDERER`, `GL_VERSION`
* `glGetError()` functionality
* `glDrawArrays` and clientside arrays
* Buffers (`glGenBuffers`, `glDeleteBuffers`, `glBindBuffer`)
* `glTexImage1D`
* `glRectf`
* `glPointSize`
* `glPostProcess()` for multithreaded post-processing
* Compile-time toggles for `GL_SELECT` and `GL_FEEDBACK`

Code Quality:
* Removed GLX/NanoGLX (not portable)
* Removed unused functions
* Fixed memory leaks (Valgrind clean)
* Viewport coordinate overflow protection
* Fixed buffer overflow in `glDrawText`
* Extensive compile-time configuration options

Note that this software rasterizer is not GL 1.1 compliant and does not constitute a complete GL implementation.

## Integration
TinyGL is not header-only. It ships C sources plus internal headers (build-only) and public headers (`gl.h`, `zfeatures.h`, `zbuffer.h`). You can build it as a static library or compile the sources directly into your program.

Basic usage:
```c
#include <GL/gl.h>
#include "zbuffer.h"

/* Open a framebuffer (pass NULL to allocate internally) */
ZBuffer *frameBuffer = ZB_open(winSizeX, winSizeY, mode, NULL);

/* Initialize TinyGL with the framebuffer */
glInit(frameBuffer);

/* Make TinyGL calls here */

/* Copy framebuffer to display (pitch = bytes per row) */
ZB_copyFrameBuffer(frameBuffer, screen->pixels, screen->pitch);

/* Clean up */
ZB_close(frameBuffer);
glClose();
```

### Requirements
* C99 compliant compiler
* 32-bit signed and unsigned integer types
* 32-bit binary float type (`STDC_IEC_559`)
* `sin` and `cos` from `<math.h>`
* `memcpy` from `<string.h>`
* `assert` from `<assert.h>` (for debugging; can be stubbed)
* Memory allocator (replacements for `malloc`, `calloc`, `free`)

SDL2 is required only for the `sdl_examples`, not for the library itself. There is no `FILE*` usage or I/O outside of `msghandling.c`; stub those calls to remove all stdio dependency.

### Multithreading Support
OpenMP is used on supported platforms to parallelize certain operations:
* `glDrawPixels` - each scanline drawn by a separate thread
* `glPostProcess` - each callback invocation runs in a separate thread
* `glCopyTexImage2D` - each scanline copied by a separate thread
* `ZB_copyBuffer` - each scanline copied by a separate thread

Compile with `-fopenmp` to enable. This is optional (disabled in `config.mk` by default) and not required to use TinyGL.

## Extension Functions
Functions not in the GL 1.1 spec, added for convenience. These cannot be added to display lists unless noted.

### glDeleteList
Simpler alternative to `glDeleteLists` (which is also implemented).

### glSetEnableSpecular(int enable)
Display-list compatible. Enable/disable specular rendering. Turn off if not using specular lighting to save cycles.

### glGetTexturePixmap(int text, int level, int* xsize, int* ysize)
Retrieve raw pixel data of a texture for modification.

### glDrawText(const unsigned char* text, int x, int y, unsigned int pixel)
Display-list compatible (as `glPlotPixel` calls). Renders 8-bit Latin extended character set using a built-in 8x8 font.

### glTextSize(GLTEXTSIZE mode)
Display-list compatible. Set the size of text drawn by `glDrawText`.

### glPlotPixel(int x, int y, unsigned int pixel)
Display-list compatible. Plot a pixel directly to the buffer.

### glGenBuffers, glDeleteBuffers, glBindBuffer, glBindBufferAsArray
Server-side buffers for clientside array data. Valid target: `GL_ARRAY_BUFFER`. See `model.c` demo for usage.

### glPostProcess(GLuint (*postprocess)(GLint x, GLint y, GLuint pixel, GLushort z))
Multithreaded post-processing. The callback receives screen coordinates (x, y), current pixel color (ARGB or 5R6G5B), and depth value (z). Returns the new pixel color. Note: take care to prevent race conditions when multithreading is enabled.

### Additional glGet Queries
Query TinyGL configuration via `glGetIntegerv`:
```c
GL_POLYGON_MAX_VERTEX    = 0xf001,
GL_MAX_BUFFERS           = 0xf002,
GL_TEXTURE_HASH_TABLE_SIZE = 0xf003,
GL_MAX_TEXTURE_LEVELS    = 0xf004,
GL_MAX_SPECULAR_BUFFERS  = 0xf005,
GL_MAX_DISPLAY_LISTS     = 0xf006,
GL_ERROR_CHECK_LEVEL     = 0xf007,
GL_IS_SPECULAR_ENABLED   = 0xf008,
```

## Build Configuration
See `include/zfeatures.h` for all compile-time options. Key settings:

Pixel format (enable exactly one):
```c
#define TGL_FEATURE_16_BITS 0  /* 5R6G5B format */
#define TGL_FEATURE_32_BITS 1  /* ARGB format */
```

Note: 32-bit output is ARGB; see SDL examples for format conversion if needed.

Other notable options:
* `TGL_FEATURE_TEXTURE_POW2` - texture dimension as power of 2 (default 8 = 256x256)
* `TGL_FEATURE_BLEND` - enable blending support
* `TGL_FEATURE_ERROR_CHECK` - enable `glGetError()` functionality
* `TGL_FEATURE_DISPLAYLISTS` - enable display list support
* `TGL_FEATURE_DIRTY_RECTANGLE` - enable dirty rectangle optimization

## Limitations
* Texture size and format are fixed at compile time (configurable in `zfeatures.h`)
* Many GL 1.1 prototypes are missing
* `glPolygonOffset` is a no-op (multiplier is 0)
* No stencil buffer
* Blending does not use alpha values; the rasterizer has no concept of alpha
* No mipmapping, antialiasing, or texture filtering
* No edge clamping; S and T coordinates are wrapped
* Infinite display list nesting will crash
* Lit triangles use current material properties even when textured (black diffuse = black textured triangles)
* Textured triangles are affected by vertex colors (call `glColor3f(1,1,1)` before rendering textured objects for expected results)
* Rendering window X dimension must be a multiple of 4
* Line rendering is not blended
* Point smoothing is not implemented (points are solid-color squares)
* `glCopyTexImage2D` only works with the compile-time texture size

## FAQ

**What can I use TinyGL for?**
- Lightweight graphics library when you cannot rely on a GPU.
- Cross-platform rendering on almost any target with IEEE 754 floats and 32-bit integers.
- Server-side rendering where you stream the framebuffer.
- Pre-rendered graphics pipelines you can customize entirely on the CPU.
- Porting projects with an OpenGL-like API to niche architectures (TinyGL is not OpenGL compliant).

**TinyGL uses too much memory**
- Tune the limits for textures, display lists, lights, and related features in `include/zgl.h` or `include/zfeatures.h`.
- Lower the texture size (e.g., 64x64 or 32x32) via `TGL_FEATURE_TEXTURE_POW2` for tighter footprints.

**How do I use 16-bit color?**
Set the feature flags in `include/zfeatures.h`:
```c
#define TGL_NO_COPY_COLOR 0xff00ff
#define TGL_NO_DRAW_COLOR 0xff00ff
#define TGL_COLOR_MASK    0x00ffffff

#define TGL_FEATURE_16_BITS 0
#define TGL_FEATURE_32_BITS 1
```
Flip `TGL_FEATURE_16_BITS` to `1` and `TGL_FEATURE_32_BITS` to `0` for 16-bit output. Adjust the NO_COPY/NO_DRAW constants to match your chosen format if you disable drawing or copying.

## Related Projects
* [PortableGL](https://github.com/rswinkle/PortableGL) - OpenGL 3.x core in clean C99 as a single header library
* [TinyGLES](https://github.com/lunixbochs/tinygles) - software OpenGL ES driver
* [Vincent ES](https://github.com/hmwill/GLESv20) - software renderer based on OpenGL ES 2.0 API
* [softgl](https://github.com/bit-hack/softgl) - software renderer based on OpenGL 1.x
* [TinyGL.js](https://github.com/humu2009/tinygl.js) - TinyGL compiled to JavaScript via Emscripten
* [scummvm/graphics/tinygl](https://github.com/scummvm/scummvm/tree/master/graphics/tinygl) - ScummVM's TinyGL fork with extensive modifications

## License
TinyGL is licensed under the MIT License. See [LICENSE](LICENSE) for details.
Note: The upstream TinyGL [changed its license](https://bellard.org/TinyGL/changelog.html) from Zlib-like to MIT in 2022.
