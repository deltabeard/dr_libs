// Public Domain. See "unlicense" statement at the end of this file.

// ABOUT
// 
// easy_2d is a simple library for drawing simple 2D graphics.
//
//
//
// USAGE
//
// This is a single-file library. To use it, do something like the following in one .c file.
//   #define EASY_2D_IMPLEMENTATION
//   #include "easy_2d.h"
//
// You can then #include easy_2d.h in other parts of the program as you would with any other header file.
//
//
//
// QUICK NOTES
//
// - Drawing must be done inside a easy2d_begin_draw() and easy2d_end_draw() pair. Rationale: 1) required for compatibility
//   with GDI's BeginPaint() and EndPaint() APIs; 2) gives implementations opportunity to save and restore state, such as
//   OpenGL state and whatnot.
// - This library is not thread safe.
//
//
//
// OPTIONS
//
// #define EASY2D_NO_GDI
//   Excludes the GDI back-end.
//
// #define EASY2D_NO_CAIRO
//   Excludes the Cairo back-end.
//
//
//
// TODO
// - Document resource management.

#ifndef easy_2d_h
#define easy_2d_h

#include <stdlib.h>
#include <stdbool.h>

#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
#include <windows.h>

// No Cairo on Win32 builds.
#ifndef EASY2D_NO_CAIRO
#define EASY2D_NO_CAIRO
#endif
#else
// No GDI on non-Win32 builds.
#ifndef EASY2D_NO_GDI
#define EASY2D_NO_GDI
#endif
#endif

#ifndef EASY2D_MAX_FONT_FAMILY_LENGTH
#define EASY2D_MAX_FONT_FAMILY_LENGTH   128
#endif


#ifdef __cplusplus
extern "C" {
#endif


/////////////////////////////////////////////////////////////////
//
// CORE 2D API
//
/////////////////////////////////////////////////////////////////

typedef unsigned char easy2d_byte;

typedef struct easy2d_context easy2d_context;
typedef struct easy2d_surface easy2d_surface;
typedef struct easy2d_font easy2d_font;
typedef struct easy2d_image easy2d_image;
typedef struct easy2d_color easy2d_color;
typedef struct easy2d_font_metrics easy2d_font_metrics;
typedef struct easy2d_glyph_metrics easy2d_glyph_metrics;
typedef struct easy2d_drawing_callbacks easy2d_drawing_callbacks;


/// Structure representing an RGBA color. Color components are specified in the range of 0 - 255.
struct easy2d_color
{
    easy2d_byte r;
    easy2d_byte g;
    easy2d_byte b;
    easy2d_byte a;
};

struct easy2d_font_metrics
{
    int ascent;
    int descent;
    int lineHeight;
    int spaceWidth;
};

struct easy2d_glyph_metrics
{
    int width;
    int height;
    int originX;
    int originY;
    int advanceX;
    int advanceY;
};

typedef enum
{
    easy2d_font_weight_medium = 0,
    easy2d_font_weight_thin,
    easy2d_font_weight_extra_light,
    easy2d_font_weight_light,
    easy2d_font_weight_semi_bold,
    easy2d_font_weight_bold,
    easy2d_font_weight_extra_bold,
    easy2d_font_weight_heavy,

    easy2d_font_weight_normal  = easy2d_font_weight_medium,
    easy2d_font_weight_default = easy2d_font_weight_medium

} easy2d_font_weight;

typedef enum
{
    easy2d_font_slant_none = 0,
    easy2d_font_slant_italic,
    easy2d_font_slant_oblique

} easy2d_font_slant;


#define EASY2D_IMAGE_DRAW_BACKGROUND    (1 << 0)
#define EASY2D_IMAGE_DRAW_BOUNDS        (1 << 1)
#define EASY2D_IMAGE_CLIP_BOUNDS        (1 << 2)        //< Clips the image to the bounds
#define EASY2D_IMAGE_ALIGN_CENTER       (1 << 3)
#define EASY2D_IMAGE_HINT_NO_ALPHA      (1 << 4)

typedef struct
{
    /// The destination position on the x axis. This is ignored if the EASY2D_IMAGE_ALIGN_CENTER option is set.
    float dstX;

    /// The destination position on the y axis. This is ignored if the EASY2D_IMAGE_ALIGN_CENTER option is set.
    float dstY;

    /// The destination width.
    float dstWidth;

    /// The destination height.
    float dstHeight;


    /// The source offset on the x axis.
    float srcX;

    /// The source offset on the y axis.
    float srcY;

    /// The source width.
    float srcWidth;

    /// The source height.
    float srcHeight;


    /// The position of the destination's bounds on the x axis.
    float dstBoundsX;

    /// The position of the destination's bounds on the y axis.
    float dstBoundsY;

    /// The width of the destination's bounds.
    float dstBoundsWidth;

    /// The height of the destination's bounds.
    float dstBoundsHeight;


    /// The foreground tint color. This is not applied to the background color, and the alpha component is ignored.
    easy2d_color foregroundTint;

    /// The background color. Only used if the EASY2D_IMAGE_DRAW_BACKGROUND option is set.
    easy2d_color backgroundColor;

    /// The bounds color. This color is used for the region of the bounds that sit on the outside of the destination rectangle. This will
    /// usually be set to the same value as backgroundColor, but it could also be used to draw a border around the image.
    easy2d_color boundsColor;


    /// Flags for controlling how the image should be drawn.
    unsigned int options;

} easy2d_draw_image_args;


typedef bool (* easy2d_on_create_context_proc)                  (easy2d_context* pContext);
typedef void (* easy2d_on_delete_context_proc)                  (easy2d_context* pContext);
typedef bool (* easy2d_on_create_surface_proc)                  (easy2d_surface* pSurface, float width, float height);
typedef void (* easy2d_on_delete_surface_proc)                  (easy2d_surface* pSurface);
typedef bool (* easy2d_on_create_font_proc)                     (easy2d_font* pFont);
typedef void (* easy2d_on_delete_font_proc)                     (easy2d_font* pFont);
typedef bool (* easy2d_on_create_image_proc)                    (easy2d_image* pImage, unsigned int stride, const void* pData);
typedef void (* easy2d_on_delete_image_proc)                    (easy2d_image* pImage);
typedef void (* easy2d_begin_draw_proc)                         (easy2d_surface* pSurface);
typedef void (* easy2d_end_draw_proc)                           (easy2d_surface* pSurface);
typedef void (* easy2d_clear_proc)                              (easy2d_surface* pSurface, easy2d_color color);
typedef void (* easy2d_draw_rect_proc)                          (easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color);
typedef void (* easy2d_draw_rect_outline_proc)                  (easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth);
typedef void (* easy2d_draw_rect_with_outline_proc)             (easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth, easy2d_color outlineColor);
typedef void (* easy2d_draw_round_rect_proc)                    (easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float width);
typedef void (* easy2d_draw_round_rect_outline_proc)            (easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float width, float outlineWidth);
typedef void (* easy2d_draw_round_rect_with_outline_proc)       (easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float width, float outlineWidth, easy2d_color outlineColor);
typedef void (* easy2d_draw_text_proc)                          (easy2d_surface* pSurface, easy2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, easy2d_color color, easy2d_color backgroundColor);
typedef void (* easy2d_draw_image_proc)                         (easy2d_surface* pSurface, easy2d_image* pImage, easy2d_draw_image_args* pArgs);
typedef void (* easy2d_set_clip_proc)                           (easy2d_surface* pSurface, float left, float top, float right, float bottom);
typedef void (* easy2d_get_clip_proc)                           (easy2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut);
typedef bool (* easy2d_get_font_metrics_proc)                   (easy2d_font* pFont, easy2d_font_metrics* pMetricsOut);
typedef bool (* easy2d_get_glyph_metrics_proc)                  (easy2d_font* pFont, unsigned int utf32, easy2d_glyph_metrics* pMetricsOut);
typedef bool (* easy2d_measure_string_proc)                     (easy2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut);
typedef bool (* easy2d_get_text_cursor_position_from_point_proc)(easy2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, unsigned int* pCharacterIndexOut);
typedef bool (* easy2d_get_text_cursor_position_from_char_proc) (easy2d_font* pFont, const char* text, unsigned int characterIndex, float* pTextCursorPosXOut);


struct easy2d_drawing_callbacks
{
    easy2d_on_create_context_proc on_create_context;
    easy2d_on_delete_context_proc on_delete_context;
    easy2d_on_create_surface_proc on_create_surface;
    easy2d_on_delete_surface_proc on_delete_surface;
    easy2d_on_create_font_proc    on_create_font;
    easy2d_on_delete_font_proc    on_delete_font;
    easy2d_on_create_image_proc   on_create_image;
    easy2d_on_delete_image_proc   on_delete_image;

    easy2d_begin_draw_proc                   begin_draw;
    easy2d_end_draw_proc                     end_draw;
    easy2d_clear_proc                        clear;
    easy2d_draw_rect_proc                    draw_rect;
    easy2d_draw_rect_outline_proc            draw_rect_outline;
    easy2d_draw_rect_with_outline_proc       draw_rect_with_outline;
    easy2d_draw_round_rect_proc              draw_round_rect;
    easy2d_draw_round_rect_outline_proc      draw_round_rect_outline;
    easy2d_draw_round_rect_with_outline_proc draw_round_rect_with_outline;
    easy2d_draw_text_proc                    draw_text;
    easy2d_draw_image_proc                   draw_image;
    easy2d_set_clip_proc                     set_clip;
    easy2d_get_clip_proc                     get_clip;

    easy2d_get_font_metrics_proc                    get_font_metrics;
    easy2d_get_glyph_metrics_proc                   get_glyph_metrics;
    easy2d_measure_string_proc                      measure_string;
    easy2d_get_text_cursor_position_from_point_proc get_text_cursor_position_from_point;
    easy2d_get_text_cursor_position_from_char_proc  get_text_cursor_position_from_char;
};

struct easy2d_image
{
    /// A pointer to the context that owns the image.
    easy2d_context* pContext;

    /// The width of the image.
    unsigned int width;

    /// The height of the image.
    unsigned int height;

    /// The extra bytes. The size of this buffer is equal to pContext->imageExtraBytes.
    easy2d_byte pExtraData[1];
};

struct easy2d_font
{
    /// A pointer to the context that owns the font.
    easy2d_context* pContext;

    /// The font family.
    char family[EASY2D_MAX_FONT_FAMILY_LENGTH];

    /// The size of the font.
    unsigned int size;

    /// The font's weight.
    easy2d_font_weight weight;

    /// The font's slant.
    easy2d_font_slant slant;

    /// The font's rotation, in degrees.
    float rotation;

    /// The extra bytes. The size of this buffer is equal to pContext->fontExtraBytes.
    easy2d_byte pExtraData[1];
};

struct easy2d_surface
{
    /// A pointer to the context that owns the surface.
    easy2d_context* pContext;

    /// The width of the surface.
    float width;

    /// The height of the surface.
    float height;

    /// The extra bytes. The size of this buffer is equal to pContext->surfaceExtraBytes.
    easy2d_byte pExtraData[1];
};

struct easy2d_context
{
    /// The drawing callbacks.
    easy2d_drawing_callbacks drawingCallbacks;

    /// The number of extra bytes to allocate for each image.
    size_t imageExtraBytes;

    /// The number of extra bytes to allocate for each font.
    size_t fontExtraBytes;

    /// The number of extra bytes to allocate for each surface.
    size_t surfaceExtraBytes;

    /// The number of extra bytes to allocate for the context.
    size_t contextExtraBytes;

    /// The extra bytes.
    easy2d_byte pExtraData[1];
};



/// Creats a context.
easy2d_context* easy2d_create_context(easy2d_drawing_callbacks drawingCallbacks, size_t contextExtraBytes, size_t surfaceExtraBytes, size_t fontExtraBytes, size_t imageExtraBytes);

/// Deletes the given context.
void easy2d_delete_context(easy2d_context* pContext);

/// Retrieves a pointer to the given context's extra data buffer.
void* easy2d_get_context_extra_data(easy2d_context* pContext);


/// Creates a surface.
easy2d_surface* easy2d_create_surface(easy2d_context* pContext, float width, float height);

/// Deletes the given surface.
void easy2d_delete_surface(easy2d_surface* pSurface);

/// Retrieves the width of the surface.
float easy2d_get_surface_width(const easy2d_surface* pSurface);

/// Retrieves the height of the surface.
float easy2d_get_surface_height(const easy2d_surface* pSurface);

/// Retrieves a pointer to the given surface's extra data buffer.
void* easy2d_get_surface_extra_data(easy2d_surface* pSurface);



//// Drawing ////

/// Marks the beginning of a paint operation.
void easy2d_begin_draw(easy2d_surface* pSurface);

/// Marks the end of a paint operation.
void easy2d_end_draw(easy2d_surface* pSurface);

/// Clears the given surface with the given color.
void easy2d_clear(easy2d_surface* pSurface, easy2d_color color);

/// Draws a filled rectangle without an outline.
void easy2d_draw_rect(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color);

/// Draws the outline of the given rectangle.
void easy2d_draw_rect_outline(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth);

/// Draws a filled rectangle with an outline.
void easy2d_draw_rect_with_outline(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth, easy2d_color outlineColor);

/// Draws a filled rectangle without an outline with rounded corners.
void easy2d_draw_round_rect(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius);

/// Draws the outline of the given rectangle with rounded corners.
void easy2d_draw_round_rect_outline(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius, float outlineWidth);

/// Draws a filled rectangle with an outline.
void easy2d_draw_round_rect_with_outline(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius, float outlineWidth, easy2d_color outlineColor);

/// Draws a run of text.
void easy2d_draw_text(easy2d_surface* pSurface, easy2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, easy2d_color color, easy2d_color backgroundColor);

/// Draws an image.
void easy2d_draw_image(easy2d_surface* pSurface, easy2d_image* pImage, easy2d_draw_image_args* pArgs);

/// Sets the clipping rectangle.
void easy2d_set_clip(easy2d_surface* pSurface, float left, float top, float right, float bottom);

/// Retrieves the clipping rectangle.
void easy2d_get_clip(easy2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut);


/// Creates a font that can be passed to easy2d_draw_text().
easy2d_font* easy2d_create_font(easy2d_context* pContext, const char* family, unsigned int size, easy2d_font_weight weight, easy2d_font_slant slant, float rotation);

/// Deletes a font that was previously created with easy2d_create_font()
void easy2d_delete_font(easy2d_font* pFont);

/// Retrieves a pointer to the given font's extra data buffer.
void* easy2d_get_font_extra_data(easy2d_font* pFont);

/// Retrieves the size of the given font.
unsigned int easy2d_get_font_size(easy2d_font* pFont);

/// Retrieves the metrics of the given font.
bool easy2d_get_font_metrics(easy2d_font* pFont, easy2d_font_metrics* pMetricsOut);

/// Retrieves the metrics of the glyph for the given character when rendered with the given font.
bool easy2d_get_glyph_metrics(easy2d_font* pFont, unsigned int utf32, easy2d_glyph_metrics* pMetricsOut);

/// Retrieves the dimensions of the given string when drawn with the given font.
bool easy2d_measure_string(easy2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut);

/// Retrieves the position to place a text cursor based on the given point for the given string when drawn with the given font.
bool easy2d_get_text_cursor_position_from_point(easy2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, unsigned int* pCharacterIndexOut);

/// Retrieves the position to palce a text cursor based on the character at the given index for the given string when drawn with the given font.
bool easy2d_get_text_cursor_position_from_char(easy2d_font* pFont, const char* text, unsigned int characterIndex, float* pTextCursorPosXOut);


/// Creates an image that can be passed to easy2d_draw_image().
///
/// @remarks
///     Images are immutable. If the data of an image needs to change, the image must be deleted and re-created.
///     @par
///     The image data must be in 32-bit, RGBA format where each component is in the range of 0 - 255.
easy2d_image* easy2d_create_image(easy2d_context* pContext, unsigned int width, unsigned int height, unsigned int stride, const void* pData);

/// Deletes the given image.
void easy2d_delete_image(easy2d_image* pImage);

/// Retrieves a pointer to the given image's extra data buffer.
void* easy2d_get_image_extra_data(easy2d_image* pImage);

/// Retrieves the size of the given image.
void easy2d_get_image_size(easy2d_image* pImage, unsigned int* pWidthOut, unsigned int* pHeightOut);


/////////////////////////////////////////////////////////////////
//
// UTILITY API
//
/////////////////////////////////////////////////////////////////

/// Creates a color object from a set of RGBA color components.
easy2d_color easy2d_rgba(easy2d_byte r, easy2d_byte g, easy2d_byte b, easy2d_byte a);

/// Creates a fully opaque color object from a set of RGB color components.
easy2d_color easy2d_rgb(easy2d_byte r, easy2d_byte g, easy2d_byte b);




/////////////////////////////////////////////////////////////////
//
// WINDOWS GDI 2D API
//
// When using GDI as the rendering back-end you will usually want to only call drawing functions in response to a WM_PAINT message.
//
/////////////////////////////////////////////////////////////////
#ifndef EASY2D_NO_GDI

/// Creates a 2D context with GDI as the backend.
easy2d_context* easy2d_create_context_gdi();

/// Creates a surface that draws directly to the given window.
///
/// @remarks
///     When using this kind of surface, the internal HBITMAP is not used.
easy2d_surface* easy2d_create_surface_gdi_HWND(easy2d_context* pContext, HWND hWnd);

/// Retrieves the internal HDC that we have been rendering to for the given surface.
///
/// @remarks
///     This assumes the given surface was created from a context that was created from easy2d_create_context_gdi()
HDC easy2d_get_HDC(easy2d_surface* pSurface);

/// Retrieves the internal HBITMAP object that we have been rendering to.
///
/// @remarks
///     This assumes the given surface was created from a context that was created from easy2d_create_context_gdi().
HBITMAP easy2d_get_HBITMAP(easy2d_surface* pSurface);

/// Retrieves the internal HFONT object from the given easy2d_font object.
HFONT easy2d_get_HFONT(easy2d_font* pFont);

#endif  // GDI



/////////////////////////////////////////////////////////////////
//
// CAIRO 2D API
//
/////////////////////////////////////////////////////////////////
#ifndef EASY2D_NO_CAIRO
#include <cairo/cairo.h>

/// Creates a 2D context with Cairo as the backend.
easy2d_context* easy2d_create_context_cairo();

/// Retrieves the internal cairo_surface_t object from the given surface.
///
/// @remarks
///     This assumes the given surface was created from a context that was created with easy2d_create_context_cairo().
cairo_surface_t* easy2d_get_cairo_surface_t(easy2d_surface* pSurface);

/// Retrieves the internal cairo_t object from the given surface.
cairo_t* easy2d_get_cairo_t(easy2d_surface* pSurface);

#endif  // Cairo


#ifdef __cplusplus
}
#endif

#endif  //easy_2d_h


///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef EASY_2D_IMPLEMENTATION
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef EASY2D_PRIVATE
#define EASY2D_PRIVATE static
#endif


easy2d_context* easy2d_create_context(easy2d_drawing_callbacks drawingCallbacks, size_t contextExtraBytes, size_t surfaceExtraBytes, size_t fontExtraBytes, size_t imageExtraBytes)
{
    easy2d_context* pContext = (easy2d_context*)malloc(sizeof(easy2d_context) - sizeof(pContext->pExtraData) + contextExtraBytes);
    if (pContext != NULL)
    {
        pContext->drawingCallbacks  = drawingCallbacks;
        pContext->imageExtraBytes   = imageExtraBytes;
        pContext->fontExtraBytes    = fontExtraBytes;
        pContext->surfaceExtraBytes = surfaceExtraBytes;
        pContext->contextExtraBytes = contextExtraBytes;
        memset(pContext->pExtraData, 0, contextExtraBytes);

        // The create_context callback is optional. If it is set to NULL, we just finish up here and return successfully. Otherwise
        // we want to call the create_context callback and check it's return value. If it's return value if false it means there
        // was an error and we need to return null.
        if (pContext->drawingCallbacks.on_create_context != NULL)
        {
            if (!pContext->drawingCallbacks.on_create_context(pContext))
            {
                // An error was thrown from the callback.
                free(pContext);
                pContext = NULL;
            }
        }
    }

    return pContext;
}

void easy2d_delete_context(easy2d_context* pContext)
{
    if (pContext != NULL) {
        if (pContext->drawingCallbacks.on_delete_context != NULL) {
            pContext->drawingCallbacks.on_delete_context(pContext);
        }

        free(pContext);
    }
}

void* easy2d_get_context_extra_data(easy2d_context* pContext)
{
    return pContext->pExtraData;
}


easy2d_surface* easy2d_create_surface(easy2d_context* pContext, float width, float height)
{
    if (pContext != NULL)
    {
        easy2d_surface* pSurface = (easy2d_surface*)malloc(sizeof(easy2d_surface) - sizeof(pContext->pExtraData) + pContext->surfaceExtraBytes);
        if (pSurface != NULL)
        {
            pSurface->pContext = pContext;
            pSurface->width    = width;
            pSurface->height   = height;
            memset(pSurface->pExtraData, 0, pContext->surfaceExtraBytes);

            // The create_surface callback is optional. If it is set to NULL, we just finish up here and return successfully. Otherwise
            // we want to call the create_surface callback and check it's return value. If it's return value if false it means there
            // was an error and we need to return null.
            if (pContext->drawingCallbacks.on_create_surface != NULL)
            {
                if (!pContext->drawingCallbacks.on_create_surface(pSurface, width, height))
                {
                    free(pSurface);
                    pSurface = NULL;
                }
            }
        }

        return pSurface;
    }

    return NULL;
}

void easy2d_delete_surface(easy2d_surface* pSurface)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.on_delete_surface != NULL) {
            pSurface->pContext->drawingCallbacks.on_delete_surface(pSurface);
        }

        free(pSurface);
    }
}

float easy2d_get_surface_width(const easy2d_surface* pSurface)
{
    if (pSurface != NULL) {
        return pSurface->width;
    }

    return 0;
}

float easy2d_get_surface_height(const easy2d_surface* pSurface)
{
    if (pSurface != NULL) {
        return pSurface->height;
    }

    return 0;
}

void* easy2d_get_surface_extra_data(easy2d_surface* pSurface)
{
    if (pSurface != NULL) {
        return pSurface->pExtraData;
    }

    return NULL;
}


void easy2d_begin_draw(easy2d_surface* pSurface)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.begin_draw != NULL) {
            pSurface->pContext->drawingCallbacks.begin_draw(pSurface);
        }
    }
}

void easy2d_end_draw(easy2d_surface* pSurface)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.end_draw != NULL) {
            pSurface->pContext->drawingCallbacks.end_draw(pSurface);
        }
    }
}

void easy2d_clear(easy2d_surface * pSurface, easy2d_color color)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.clear != NULL) {
            pSurface->pContext->drawingCallbacks.clear(pSurface, color);
        }
    }
}

void easy2d_draw_rect(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_rect != NULL) {
            pSurface->pContext->drawingCallbacks.draw_rect(pSurface, left, top, right, bottom, color);
        }
    }
}

void easy2d_draw_rect_outline(easy2d_surface * pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_rect_outline != NULL) {
            pSurface->pContext->drawingCallbacks.draw_rect_outline(pSurface, left, top, right, bottom, color, outlineWidth);
        }
    }
}

void easy2d_draw_rect_with_outline(easy2d_surface * pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth, easy2d_color outlineColor)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_rect_with_outline != NULL) {
            pSurface->pContext->drawingCallbacks.draw_rect_with_outline(pSurface, left, top, right, bottom, color, outlineWidth, outlineColor);
        }
    }
}

void easy2d_draw_round_rect(easy2d_surface * pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_round_rect != NULL) {
            pSurface->pContext->drawingCallbacks.draw_round_rect(pSurface, left, top, right, bottom, color, radius);
        }
    }
}

void easy2d_draw_round_rect_outline(easy2d_surface * pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius, float outlineWidth)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_round_rect_outline != NULL) {
            pSurface->pContext->drawingCallbacks.draw_round_rect_outline(pSurface, left, top, right, bottom, color, radius, outlineWidth);
        }
    }
}

void easy2d_draw_round_rect_with_outline(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius, float outlineWidth, easy2d_color outlineColor)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_round_rect_with_outline != NULL) {
            pSurface->pContext->drawingCallbacks.draw_round_rect_with_outline(pSurface, left, top, right, bottom, color, radius, outlineWidth, outlineColor);
        }
    }
}

void easy2d_draw_text(easy2d_surface* pSurface, easy2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, easy2d_color color, easy2d_color backgroundColor)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_text != NULL) {
            pSurface->pContext->drawingCallbacks.draw_text(pSurface, pFont, text, textSizeInBytes, posX, posY, color, backgroundColor);
        }
    }
}

void easy2d_draw_image(easy2d_surface* pSurface, easy2d_image* pImage, easy2d_draw_image_args* pArgs)
{
    if (pSurface == NULL || pImage == NULL || pArgs == NULL) {
        return;
    }

    assert(pSurface->pContext != NULL);

    if (pSurface->pContext->drawingCallbacks.draw_image) {
        pSurface->pContext->drawingCallbacks.draw_image(pSurface, pImage, pArgs);
    }
}

void easy2d_set_clip(easy2d_surface* pSurface, float left, float top, float right, float bottom)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.set_clip != NULL) {
            pSurface->pContext->drawingCallbacks.set_clip(pSurface, left, top, right, bottom);
        }
    }
}

void easy2d_get_clip(easy2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.get_clip != NULL) {
            pSurface->pContext->drawingCallbacks.get_clip(pSurface, pLeftOut, pTopOut, pRightOut, pBottomOut);
        }
    }
}

easy2d_font* easy2d_create_font(easy2d_context* pContext, const char* family, unsigned int size, easy2d_font_weight weight, easy2d_font_slant slant, float rotation)
{
    if (pContext == NULL) {
        return NULL;
    }

    easy2d_font* pFont = malloc(sizeof(easy2d_font) - sizeof(pFont->pExtraData) + pContext->fontExtraBytes);
    if (pFont == NULL) {
        return NULL;
    }

    pFont->pContext  = pContext;
    pFont->family[0] = '\0';
    pFont->size      = size;
    pFont->weight    = weight;
    pFont->slant     = slant;
    pFont->rotation  = rotation;

    if (family != NULL) {
        strcpy_s(pFont->family, sizeof(pFont->family), family);
    }

    if (pContext->drawingCallbacks.on_create_font != NULL) {
        if (!pContext->drawingCallbacks.on_create_font(pFont)) {
            free(pFont);
            return NULL;
        }
    }

    return pFont;
}

void easy2d_delete_font(easy2d_font* pFont)
{
    if (pFont == NULL) {
        return;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.on_delete_font != NULL) {
        pFont->pContext->drawingCallbacks.on_delete_font(pFont);
    }

    free(pFont);
}

void* easy2d_get_font_extra_data(easy2d_font* pFont)
{
    if (pFont == NULL) {
        return NULL;
    }

    return pFont->pExtraData;
}

unsigned int easy2d_get_font_size(easy2d_font* pFont)
{
    if (pFont == NULL) {
        return 0;
    }

    return pFont->size;
}

bool easy2d_get_font_metrics(easy2d_font* pFont, easy2d_font_metrics* pMetricsOut)
{
    if (pFont == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.get_font_metrics != NULL) {
        return pFont->pContext->drawingCallbacks.get_font_metrics(pFont, pMetricsOut);
    }

    return false;
}

bool easy2d_get_glyph_metrics(easy2d_font* pFont, unsigned int utf32, easy2d_glyph_metrics* pMetricsOut)
{
    if (pFont == NULL || pMetricsOut == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.get_glyph_metrics != NULL) {
        return pFont->pContext->drawingCallbacks.get_glyph_metrics(pFont, utf32, pMetricsOut);
    }

    return false;
}

bool easy2d_measure_string(easy2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut)
{
    if (pFont == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.measure_string != NULL) {
        return pFont->pContext->drawingCallbacks.measure_string(pFont, text, textSizeInBytes, pWidthOut, pHeightOut);
    }

    return false;
}

bool easy2d_get_text_cursor_position_from_point(easy2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, unsigned int* pCharacterIndexOut)
{
    if (pFont == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.get_text_cursor_position_from_point) {
        return pFont->pContext->drawingCallbacks.get_text_cursor_position_from_point(pFont, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut);
    }

    return false;
}

bool easy2d_get_text_cursor_position_from_char(easy2d_font* pFont, const char* text, unsigned int characterIndex, float* pTextCursorPosXOut)
{
    if (pFont == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.get_text_cursor_position_from_char) {
        return pFont->pContext->drawingCallbacks.get_text_cursor_position_from_char(pFont, text, characterIndex, pTextCursorPosXOut);
    }

    return false;
}


easy2d_image* easy2d_create_image(easy2d_context* pContext, unsigned int width, unsigned int height, unsigned int stride, const void* pData)
{
    if (pContext == NULL || width == 0 || height == 0 || pData == NULL) {
        return NULL;
    }

    easy2d_image* pImage = malloc(sizeof(easy2d_image) - sizeof(pImage->pExtraData) + pContext->imageExtraBytes);
    if (pImage == NULL) {
        return NULL;
    }

    pImage->pContext = pContext;
    pImage->width    = width;
    pImage->height   = height;

    if (pContext->drawingCallbacks.on_create_image != NULL) {
        if (!pContext->drawingCallbacks.on_create_image(pImage, stride, pData)) {
            free(pImage);
            return NULL;
        }
    }

    return pImage;
}

void easy2d_delete_image(easy2d_image* pImage)
{
    if (pImage == NULL) {
        return;
    }

    assert(pImage->pContext != NULL);

    if (pImage->pContext->drawingCallbacks.on_delete_image != NULL) {
        pImage->pContext->drawingCallbacks.on_delete_image(pImage);
    }

    free(pImage);
}

void* easy2d_get_image_extra_data(easy2d_image* pImage)
{
    if (pImage == NULL) {
        return NULL;
    }

    return pImage->pExtraData;
}

void easy2d_get_image_size(easy2d_image* pImage, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    if (pImage == NULL) {
        return;
    }

    if (pWidthOut) {
        *pWidthOut = pImage->width;
    }
    if (pHeightOut) {
        *pHeightOut = pImage->height;
    }
}



/////////////////////////////////////////////////////////////////
//
// UTILITY API
//
/////////////////////////////////////////////////////////////////

easy2d_color easy2d_rgba(easy2d_byte r, easy2d_byte g, easy2d_byte b, easy2d_byte a)
{
    easy2d_color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    return color;
}

easy2d_color easy2d_rgb(easy2d_byte r, easy2d_byte g, easy2d_byte b)
{
    easy2d_color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 255;

    return color;
}




/////////////////////////////////////////////////////////////////
//
// WINDOWS GDI 2D API
//
/////////////////////////////////////////////////////////////////
#ifndef EASY2D_NO_GDI

typedef struct
{
    /// The device context that owns every surface HBITMAP object. All drawing is done through this DC.
    HDC hDC;

    /// The buffer used to store wchar strings when converting from char* to wchar_t* strings. We just use a global buffer for
    /// this so we can avoid unnecessary allocations.
    wchar_t* wcharBuffer;

    /// The size of wcharBuffer (including the null terminator).
    unsigned int wcharBufferLength;

} gdi_context_data;

typedef struct
{
    /// The window to draw to. The can be null, which is the case when creating the surface with easy2d_create_surface(). When this
    /// is non-null the size of the surface is always tied to the window.
    HWND hWnd;

    /// The HDC to use when drawing to the surface.
    HDC hDC;

    /// The intermediate DC to use when drawing bitmaps.
    HDC hIntermediateDC;

    /// The PAINTSTRUCT object that is filled by BeginPaint(). Only used when hWnd is non-null.
    PAINTSTRUCT ps;

    /// The bitmap to render to. This is created with GDI's CreateDIBSection() API, using the DC above. This is only used
    /// when hDC is NULL, which is the default.
    HBITMAP hBitmap;

    /// A pointer to the raw bitmap data. This is allocated CreateDIBSection().
    void* pBitmapData;


    /// The stock DC brush.
    HGDIOBJ hStockDCBrush;

    /// The stock null brush.
    HGDIOBJ hStockNullBrush;

    /// The stock DC pen.
    HGDIOBJ hStockDCPen;

    /// The stock null pen.
    HGDIOBJ hStockNullPen;


    /// The pen that was active at the start of drawing. This is restored at the end of drawing.
    HGDIOBJ hPrevPen;

    /// The brush that was active at the start of drawing.
    HGDIOBJ hPrevBrush;

    /// The brush color at the start of drawing.
    COLORREF prevBrushColor;

    /// The previous font.
    HGDIOBJ hPrevFont;

    /// The previous text background mode.
    int prevBkMode;

    /// The previous text background color.
    COLORREF prevBkColor;


} gdi_surface_data;

typedef struct
{
    /// A handle to the Win32 font.
    HFONT hFont;

    /// The font metrics for faster retrieval. We cache the metrics when the font is loaded.
    easy2d_font_metrics metrics;

} gdi_font_data;

typedef struct
{
    /// A handle to the primary bitmap object.
    HBITMAP hSrcBitmap;

    /// A pointer to the raw bitmap data.
    unsigned int* pSrcBitmapData;

    /// A handle to the secondary bitmap object that we use when we need to change the color values of
    /// the primary bitmap before drawing.
    HBITMAP hIntermediateBitmap;

    /// A pointer to the raw data of the intermediate bitmap.
    unsigned int* pIntermediateBitmapData;

} gdi_image_data;


bool easy2d_on_create_context_gdi(easy2d_context* pContext);
void easy2d_on_delete_context_gdi(easy2d_context* pContext);
bool easy2d_on_create_surface_gdi(easy2d_surface* pSurface, float width, float height);
void easy2d_on_delete_surface_gdi(easy2d_surface* pSurface);
bool easy2d_on_create_font_gdi(easy2d_font* pFont);
void easy2d_on_delete_font_gdi(easy2d_font* pFont);
bool easy2d_on_create_image_gdi(easy2d_image* pImage, unsigned int stride, const void* pData);
void easy2d_on_delete_image_gdi(easy2d_image* pImage);

void easy2d_begin_draw_gdi(easy2d_surface* pSurface);
void easy2d_end_draw_gdi(easy2d_surface* pSurface);
void easy2d_clear_gdi(easy2d_surface* pSurface, easy2d_color color);
void easy2d_draw_rect_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color);
void easy2d_draw_rect_outline_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth);
void easy2d_draw_rect_with_outline_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth, easy2d_color outlineColor);
void easy2d_draw_round_rect_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius);
void easy2d_draw_round_rect_outline_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius, float outlineWidth);
void easy2d_draw_round_rect_with_outline_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius, float outlineWidth, easy2d_color outlineColor);
void easy2d_draw_text_gdi(easy2d_surface* pSurface, easy2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, easy2d_color color, easy2d_color backgroundColor);
void easy2d_draw_image_gdi(easy2d_surface* pSurface, easy2d_image* pImage, easy2d_draw_image_args* pArgs);
void easy2d_set_clip_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom);
void easy2d_get_clip_gdi(easy2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut);

bool easy2d_get_font_metrics_gdi(easy2d_font* pFont, easy2d_font_metrics* pMetricsOut);
bool easy2d_get_glyph_metrics_gdi(easy2d_font* pFont, unsigned int utf32, easy2d_glyph_metrics* pGlyphMetrics);
bool easy2d_measure_string_gdi(easy2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut);
bool easy2d_get_text_cursor_position_from_point_gdi(easy2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, unsigned int* pCharacterIndexOut);
bool easy2d_get_text_cursor_position_from_char_gdi(easy2d_font* pFont, const char* text, unsigned int characterIndex, float* pTextCursorPosXOut);

/// Converts a char* to a wchar_t* string.
wchar_t* easy2d_to_wchar_gdi(easy2d_context* pContext, const char* text, size_t textSizeInBytes, unsigned int* characterCountOut);

/// Converts a UTF-32 character to a UTF-16.
static int easy2d_utf32_to_utf16(unsigned int utf32, unsigned short utf16[2])
{
    if (utf16 == NULL) {
        return 0;
    }

    if (utf32 < 0xD800 || (utf32 >= 0xE000 && utf32 <= 0xFFFF))
    {
        utf16[0] = (unsigned short)utf32;
        utf16[1] = 0;
        return 1;
    }
    else
    {
        if (utf32 >= 0x10000 && utf32 <= 0x10FFFF)
        {
            utf16[0] = (unsigned short)(0xD7C0 + (unsigned short)(utf32 >> 10));
            utf16[1] = (unsigned short)(0xDC00 + (unsigned short)(utf32 & 0x3FF));
            return 2;
        }
        else
        {
            // Invalid.
            utf16[0] = 0;
            utf16[0] = 0;
            return 0;
        }
    }
}

easy2d_context* easy2d_create_context_gdi()
{
    easy2d_drawing_callbacks callbacks;
    callbacks.on_create_context                   = easy2d_on_create_context_gdi;
    callbacks.on_delete_context                   = easy2d_on_delete_context_gdi;
    callbacks.on_create_surface                   = easy2d_on_create_surface_gdi;
    callbacks.on_delete_surface                   = easy2d_on_delete_surface_gdi;
    callbacks.on_create_font                      = easy2d_on_create_font_gdi;
    callbacks.on_delete_font                      = easy2d_on_delete_font_gdi;
    callbacks.on_create_image                     = easy2d_on_create_image_gdi;
    callbacks.on_delete_image                     = easy2d_on_delete_image_gdi;

    callbacks.begin_draw                          = easy2d_begin_draw_gdi;
    callbacks.end_draw                            = easy2d_end_draw_gdi;
    callbacks.clear                               = easy2d_clear_gdi;
    callbacks.draw_rect                           = easy2d_draw_rect_gdi;
    callbacks.draw_rect_outline                   = easy2d_draw_rect_outline_gdi;
    callbacks.draw_rect_with_outline              = easy2d_draw_rect_with_outline_gdi;
    callbacks.draw_round_rect                     = easy2d_draw_round_rect_gdi;
    callbacks.draw_round_rect_outline             = easy2d_draw_round_rect_outline_gdi;
    callbacks.draw_round_rect_with_outline        = easy2d_draw_round_rect_with_outline_gdi;
    callbacks.draw_text                           = easy2d_draw_text_gdi;
    callbacks.draw_image                          = easy2d_draw_image_gdi;
    callbacks.set_clip                            = easy2d_set_clip_gdi;
    callbacks.get_clip                            = easy2d_get_clip_gdi;

    callbacks.get_font_metrics                    = easy2d_get_font_metrics_gdi;
    callbacks.get_glyph_metrics                   = easy2d_get_glyph_metrics_gdi;
    callbacks.measure_string                      = easy2d_measure_string_gdi;
    callbacks.get_text_cursor_position_from_point = easy2d_get_text_cursor_position_from_point_gdi;
    callbacks.get_text_cursor_position_from_char  = easy2d_get_text_cursor_position_from_char_gdi;

    return easy2d_create_context(callbacks, sizeof(gdi_context_data), sizeof(gdi_surface_data), sizeof(gdi_font_data), sizeof(gdi_image_data));
}

easy2d_surface* easy2d_create_surface_gdi_HWND(easy2d_context* pContext, HWND hWnd)
{
    easy2d_surface* pSurface = easy2d_create_surface(pContext, 0, 0);
    if (pSurface != NULL) {
        gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
        if (pGDIData != NULL) {
            pGDIData->hWnd = hWnd;
        }
    }

    return pSurface;
}

HDC easy2d_get_HDC(easy2d_surface* pSurface)
{
    if (pSurface != NULL) {
        gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
        if (pGDIData != NULL) {
            return pGDIData->hDC;
        }
    }

    return NULL;
}

HBITMAP easy2d_get_HBITMAP(easy2d_surface* pSurface)
{
    if (pSurface != NULL) {
        gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
        if (pGDIData != NULL) {
            return pGDIData->hBitmap;
        }
    }

    return NULL;
}

HFONT easy2d_get_HFONT(easy2d_font* pFont)
{
    gdi_font_data* pGDIData = easy2d_get_font_extra_data(pFont);
    if (pGDIData == NULL) {
        return NULL;
    }

    return pGDIData->hFont;
}


bool easy2d_on_create_context_gdi(easy2d_context* pContext)
{
    assert(pContext != NULL);

    // We need to create the DC that all of our rendering commands will be drawn to.
    gdi_context_data* pGDIData = easy2d_get_context_extra_data(pContext);
    if (pGDIData == NULL) {
        return false;
    }

    pGDIData->hDC = CreateCompatibleDC(GetDC(GetDesktopWindow()));
    if (pGDIData->hDC == NULL) {
        return false;
    }


    // We want to use the advanced graphics mode so that GetTextExtentPoint32() performs the conversions for font rotation for us.
    SetGraphicsMode(pGDIData->hDC, GM_ADVANCED);


    pGDIData->wcharBuffer       = NULL;
    pGDIData->wcharBufferLength = 0;


    return true;
}

void easy2d_on_delete_context_gdi(easy2d_context* pContext)
{
    assert(pContext != NULL);

    gdi_context_data* pGDIData = easy2d_get_context_extra_data(pContext);
    if (pGDIData != NULL)
    {
        free(pGDIData->wcharBuffer);
        pGDIData->wcharBuffer       = 0;
        pGDIData->wcharBufferLength = 0;

        DeleteDC(pGDIData->hDC);
        pGDIData->hDC = NULL;
    }
}

bool easy2d_on_create_surface_gdi(easy2d_surface* pSurface, float width, float height)
{
    assert(pSurface != NULL);

    gdi_context_data* pGDIContextData = easy2d_get_context_extra_data(pSurface->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }

    gdi_surface_data* pGDISurfaceData = easy2d_get_surface_extra_data(pSurface);
    if (pGDISurfaceData == NULL) {
        return false;
    }

    HDC hDC = pGDIContextData->hDC;
    if (hDC == NULL) {
        return false;
    }

    HDC hIntermediateDC = CreateCompatibleDC(hDC);
    if (hIntermediateDC == NULL) {
        return false;
    }

    pGDISurfaceData->hIntermediateDC = hIntermediateDC;
    pGDISurfaceData->hWnd = NULL;
    

    if (width != 0 && height != 0)
    {
        pGDISurfaceData->hDC = hDC;

        BITMAPINFO bmi;
        ZeroMemory(&bmi, sizeof(bmi));
        bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
        bmi.bmiHeader.biWidth       = (LONG)width;
        bmi.bmiHeader.biHeight      = (LONG)height;
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        pGDISurfaceData->hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pGDISurfaceData->pBitmapData, NULL, 0);
        if (pGDISurfaceData->hBitmap == NULL) {
            return false;
        }
    }
    else
    {
        pGDISurfaceData->hBitmap = NULL;
        pGDISurfaceData->hDC     = NULL;
    }


    return true;
}

void easy2d_on_delete_surface_gdi(easy2d_surface* pSurface)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        DeleteObject(pGDIData->hBitmap);
        pGDIData->hBitmap = NULL;

        DeleteDC(pGDIData->hIntermediateDC);
        pGDIData->hIntermediateDC = NULL;
    }
}

bool easy2d_on_create_font_gdi(easy2d_font* pFont)
{
    assert(pFont != NULL);

    gdi_font_data* pGDIData = easy2d_get_font_extra_data(pFont);
    if (pGDIData == NULL) {
        return false;
    }


    LONG weightGDI = FW_REGULAR;
    switch (pFont->weight)
    {
    case easy2d_font_weight_medium:      weightGDI = FW_MEDIUM;     break;
    case easy2d_font_weight_thin:        weightGDI = FW_THIN;       break;
    case easy2d_font_weight_extra_light: weightGDI = FW_EXTRALIGHT; break;
    case easy2d_font_weight_light:       weightGDI = FW_LIGHT;      break;
    case easy2d_font_weight_semi_bold:   weightGDI = FW_SEMIBOLD;   break;
    case easy2d_font_weight_bold:        weightGDI = FW_BOLD;       break;
    case easy2d_font_weight_extra_bold:  weightGDI = FW_EXTRABOLD;  break;
    case easy2d_font_weight_heavy:       weightGDI = FW_HEAVY;      break;
    default: break;
    }

	BYTE slantGDI = FALSE;
    if (pFont->slant == easy2d_font_slant_italic || pFont->slant == easy2d_font_slant_oblique) {
        slantGDI = TRUE;
    }


	LOGFONTA logfont;
	memset(&logfont, 0, sizeof(logfont));


    
    logfont.lfHeight      = -(LONG)pFont->size;
	logfont.lfWeight      = weightGDI;
	logfont.lfItalic      = slantGDI;
	logfont.lfCharSet     = DEFAULT_CHARSET;
	logfont.lfQuality     = (pFont->size > 36) ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY;
    logfont.lfEscapement  = (LONG)pFont->rotation * 10;
    logfont.lfOrientation = (LONG)pFont->rotation * 10;
    
    size_t familyLength = strlen(pFont->family);
	memcpy(logfont.lfFaceName, pFont->family, (familyLength < 31) ? familyLength : 31);


	pGDIData->hFont = CreateFontIndirectA(&logfont);
    if (pGDIData->hFont == NULL) {
        return false;
    }


    gdi_context_data* pGDIContextData = easy2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }

    // Cache the font metrics.
    HGDIOBJ hPrevFont = SelectObject(pGDIContextData->hDC, pGDIData->hFont);
    {
        TEXTMETRIC metrics;
        GetTextMetrics(pGDIContextData->hDC, &metrics);

        pGDIData->metrics.ascent     = metrics.tmAscent;
        pGDIData->metrics.descent    = metrics.tmDescent;
        pGDIData->metrics.lineHeight = metrics.tmHeight;


        const MAT2 transform = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};        // <-- Identity matrix

        GLYPHMETRICS spaceMetrics;
        DWORD bitmapBufferSize = GetGlyphOutlineW(pGDIContextData->hDC, ' ', GGO_NATIVE, &spaceMetrics, 0, NULL, &transform);
        if (bitmapBufferSize == GDI_ERROR) {
			pGDIData->metrics.spaceWidth = 4;
        } else {
            pGDIData->metrics.spaceWidth = spaceMetrics.gmCellIncX;
        }
    }
    SelectObject(pGDIContextData->hDC, hPrevFont);


    return true;
}

void easy2d_on_delete_font_gdi(easy2d_font* pFont)
{
    assert(pFont != NULL);

    gdi_font_data* pGDIData = easy2d_get_font_extra_data(pFont);
    if (pGDIData == NULL) {
        return;
    }

    DeleteObject(pGDIData->hFont);
}

bool easy2d_on_create_image_gdi(easy2d_image* pImage, unsigned int stride, const void* pData)
{
    assert(pImage != NULL);
    
    gdi_image_data* pGDIData = easy2d_get_image_extra_data(pImage);
    if (pGDIData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = easy2d_get_context_extra_data(pImage->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth       = pImage->width;
    bmi.bmiHeader.biHeight      = pImage->height;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;   // Only supporting 32-bit formats.
    bmi.bmiHeader.biCompression = BI_RGB;
    pGDIData->hSrcBitmap = CreateDIBSection(pGDIContextData->hDC, &bmi, DIB_RGB_COLORS, &pGDIData->pSrcBitmapData, NULL, 0);
    if (pGDIData->hSrcBitmap == NULL) {
        return false;
    }

    pGDIData->hIntermediateBitmap = CreateDIBSection(pGDIContextData->hDC, &bmi, DIB_RGB_COLORS, &pGDIData->pIntermediateBitmapData, NULL, 0);
    if (pGDIData->hIntermediateBitmap == NULL) {
        DeleteObject(pGDIData->hSrcBitmap);
        return false;
    }


    // We need to convert the data so it renders correctly with AlphaBlend().
    for (unsigned int iRow = 0; iRow < pImage->height; ++iRow)
    {
        const unsigned int iRowSrc = pImage->height - (iRow + 1);
        const unsigned int iRowDst = iRow;

        for (unsigned int iCol = 0; iCol < pImage->width; ++iCol)
        {
            unsigned int  srcTexel = ((const unsigned int*)(pData                   ))[  (iRowSrc * (stride/4))    + iCol];
            unsigned int* dstTexel = ((      unsigned int*)(pGDIData->pSrcBitmapData)) + (iRowDst * pImage->width) + iCol;

            unsigned int srcTexelA = (srcTexel & 0xFF000000) >> 24;
            unsigned int srcTexelB = (srcTexel & 0x00FF0000) >> 16;
            unsigned int srcTexelG = (srcTexel & 0x0000FF00) >> 8;
            unsigned int srcTexelR = (srcTexel & 0x000000FF) >> 0;

            srcTexelB = (unsigned int)(srcTexelB * (srcTexelA / 255.0f));
            srcTexelG = (unsigned int)(srcTexelG * (srcTexelA / 255.0f));
            srcTexelR = (unsigned int)(srcTexelR * (srcTexelA / 255.0f));

            *dstTexel = (srcTexelR << 16) | (srcTexelG << 8) | (srcTexelB << 0) | (srcTexelA << 24);
        }
    }

    // Flush GDI to let it know we are finished with the bitmap object's data.
    GdiFlush();

    // At this point everything should be good.
    return true;
}

void easy2d_on_delete_image_gdi(easy2d_image* pImage)
{
    assert(pImage != NULL);

    gdi_image_data* pGDIData = easy2d_get_image_extra_data(pImage);
    if (pGDIData == NULL) {
        return;
    }

    DeleteObject(pGDIData->hSrcBitmap);
    pGDIData->hSrcBitmap = NULL;

    DeleteObject(pGDIData->hIntermediateBitmap);
    pGDIData->hIntermediateBitmap = NULL;
}


void easy2d_begin_draw_gdi(easy2d_surface* pSurface)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL) {
        if (pGDIData->hWnd != NULL) {
            pGDIData->hDC = BeginPaint(pGDIData->hWnd, &pGDIData->ps);
        } else {
            SelectObject(easy2d_get_HDC(pSurface), pGDIData->hBitmap);
        }

        HDC hDC = easy2d_get_HDC(pSurface);

        pGDIData->hStockDCBrush   = GetStockObject(DC_BRUSH);
        pGDIData->hStockNullBrush = GetStockObject(NULL_BRUSH);
        pGDIData->hStockDCPen     = GetStockObject(DC_PEN);
        pGDIData->hStockNullPen   = GetStockObject(NULL_PEN);
        
        // Retrieve the defaults so they can be restored later.
        pGDIData->hPrevPen       = GetCurrentObject(hDC, OBJ_PEN);
        pGDIData->hPrevBrush     = GetCurrentObject(hDC, OBJ_BRUSH);
        pGDIData->prevBrushColor = GetDCBrushColor(hDC);
        pGDIData->hPrevFont      = GetCurrentObject(hDC, OBJ_FONT);
        pGDIData->prevBkMode     = GetBkMode(hDC);
        pGDIData->prevBkColor    = GetBkColor(hDC);
    }
}

void easy2d_end_draw_gdi(easy2d_surface* pSurface)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL) {
        HDC hDC = easy2d_get_HDC(pSurface);

        SelectClipRgn(hDC, NULL);

        SelectObject(hDC, pGDIData->hPrevPen);
        SelectObject(hDC, pGDIData->hPrevBrush);
        SetDCBrushColor(hDC, pGDIData->prevBrushColor);
        SelectObject(hDC, pGDIData->hPrevFont);
        SetBkMode(hDC, pGDIData->prevBkMode);
        SetBkColor(hDC, pGDIData->prevBkColor);

        if (pGDIData->hWnd != NULL) {
            EndPaint(pGDIData->hWnd, &pGDIData->ps);
        }
    }
}

void easy2d_clear_gdi(easy2d_surface* pSurface, easy2d_color color)
{
    assert(pSurface != NULL);

    easy2d_draw_rect_gdi(pSurface, 0, 0, pSurface->width, pSurface->height, color);
}

void easy2d_draw_rect_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = easy2d_get_HDC(pSurface);

        SelectObject(hDC, pGDIData->hStockNullPen);
        SelectObject(hDC, pGDIData->hStockDCBrush);
        SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));

        // Now draw the rectangle. The documentation for this says that the width and height is 1 pixel less when the pen is null. Therefore we will
        // increase the width and height by 1 since we have got the pen set to null.
        Rectangle(hDC, (int)left, (int)top, (int)right + 1, (int)bottom + 1);
    }
}

void easy2d_draw_rect_outline_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = easy2d_get_HDC(pSurface);

        HPEN hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, (int)outlineWidth, RGB(color.r, color.g, color.b));
        if (hPen != NULL)
        {
            SelectObject(hDC, pGDIData->hStockNullBrush);
            SelectObject(hDC, hPen);

            Rectangle(hDC, (int)left, (int)top, (int)right, (int)bottom);

            DeleteObject(hPen);
        }
    }
}

void easy2d_draw_rect_with_outline_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float outlineWidth, easy2d_color outlineColor)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = easy2d_get_HDC(pSurface);

        HPEN hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, (int)outlineWidth, RGB(outlineColor.r, outlineColor.g, outlineColor.b));
        if (hPen != NULL)
        {
            SelectObject(hDC, hPen);
            SelectObject(hDC, pGDIData->hStockDCBrush);
            SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));
            
            Rectangle(hDC, (int)left, (int)top, (int)right, (int)bottom);

            DeleteObject(hPen);
        }
    }
}

void easy2d_draw_round_rect_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = easy2d_get_HDC(pSurface);

        SelectObject(hDC, pGDIData->hStockNullPen);
        SelectObject(hDC, pGDIData->hStockDCBrush);
        SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));

        RoundRect(hDC, (int)left, (int)top, (int)right + 1, (int)bottom + 1, (int)(radius*2), (int)(radius*2));
    }
}

void easy2d_draw_round_rect_outline_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius, float outlineWidth)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = easy2d_get_HDC(pSurface);

        HPEN hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, (int)outlineWidth, RGB(color.r, color.g, color.b));
        if (hPen != NULL)
        {
            SelectObject(hDC, pGDIData->hStockNullBrush);
            SelectObject(hDC, hPen);

            RoundRect(hDC, (int)left, (int)top, (int)right, (int)bottom, (int)(radius*2), (int)(radius*2));

            DeleteObject(hPen);
        }
    }
}

void easy2d_draw_round_rect_with_outline_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color, float radius, float outlineWidth, easy2d_color outlineColor)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = easy2d_get_HDC(pSurface);

        HPEN hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, (int)outlineWidth, RGB(outlineColor.r, outlineColor.g, outlineColor.b));
        if (hPen != NULL)
        {
            SelectObject(hDC, hPen);
            SelectObject(hDC, pGDIData->hStockDCBrush);
            SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));

            RoundRect(hDC, (int)left, (int)top, (int)right, (int)bottom, (int)(radius*2), (int)(radius*2));

            DeleteObject(hPen);
        }
    }
}

void easy2d_draw_text_gdi(easy2d_surface* pSurface, easy2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, easy2d_color color, easy2d_color backgroundColor)
{
    gdi_font_data* pGDIFontData = easy2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return;
    }


    HDC hDC = easy2d_get_HDC(pSurface);

    HFONT hFontGDI = pGDIFontData->hFont;
    if (hFontGDI != NULL)
    {
        // We actually want to use the W version of TextOut because otherwise unicode doesn't work properly.

        unsigned int textWLength;
        wchar_t* textW = easy2d_to_wchar_gdi(pSurface->pContext, text, textSizeInBytes, &textWLength);
        if (textW != NULL)
        {
            SelectObject(hDC, hFontGDI);

            UINT options = 0;
            RECT rect = {0, 0, 0, 0};

            if (backgroundColor.a == 0) {
                SetBkMode(hDC, TRANSPARENT);
            } else {
                SetBkMode(hDC, OPAQUE);
                SetBkColor(hDC, RGB(backgroundColor.r, backgroundColor.g, backgroundColor.b));

                // There is an issue with the way GDI draws the background of a string of text. When ClearType is enabled, the rectangle appears
                // to be wider than it is supposed to be. As a result, drawing text right next to each other results in the most recent one being
                // drawn slightly on top of the previous one. To fix this we need to use ExtTextOut() with the ETO_CLIPPED parameter enabled.
                options |= ETO_CLIPPED;

                SIZE textSize = {0, 0};
                GetTextExtentPoint32W(hDC, textW, textWLength, &textSize);
                rect.left   = (LONG)posX;
                rect.top    = (LONG)posY;
                rect.right  = (LONG)(posX + textSize.cx);
                rect.bottom = (LONG)(posY + textSize.cy);
            }
                
            SetTextColor(hDC, RGB(color.r, color.g, color.b));

            ExtTextOutW(hDC, (int)posX, (int)posY, options, &rect, textW, textWLength, NULL);
        }
    }
}

void easy2d_draw_image_gdi(easy2d_surface* pSurface, easy2d_image* pImage, easy2d_draw_image_args* pArgs)
{
    gdi_image_data* pGDIImageData = easy2d_get_image_extra_data(pImage);
    if (pGDIImageData == NULL) {
        return;
    }

    gdi_surface_data* pGDISurfaceData = easy2d_get_surface_extra_data(pSurface);
    if (pGDISurfaceData == NULL) {
        return;
    }


    HBITMAP hSrcBitmap = NULL;

    // Center the image if applicable.
    if ((pArgs->options & EASY2D_IMAGE_ALIGN_CENTER) != 0)
    {
        pArgs->dstX = pArgs->dstBoundsX + (pArgs->dstBoundsWidth  - pArgs->dstWidth)  / 2;
        pArgs->dstY = pArgs->dstBoundsY + (pArgs->dstBoundsHeight - pArgs->dstHeight) / 2;
    }

    // Clip the image if applicable.
    int prevDC = 0;
    if ((pArgs->options & EASY2D_IMAGE_CLIP_BOUNDS) != 0)
    {
        // We only need to clip if part of the destination rectangle falls outside of the bounds.
        if (pArgs->dstX < pArgs->dstBoundsX || pArgs->dstX + pArgs->dstWidth  > pArgs->dstBoundsX + pArgs->dstBoundsWidth ||
            pArgs->dstY < pArgs->dstBoundsY || pArgs->dstY + pArgs->dstHeight > pArgs->dstBoundsY + pArgs->dstBoundsHeight)
        {
            if (pArgs->dstWidth != pArgs->srcWidth || pArgs->dstHeight != pArgs->srcHeight)
            {
                prevDC = SaveDC(pGDISurfaceData->hDC);
            }
            else
            {
                // There's no scaling so we can do a more efficient clip by simply adjusting the source and destination rectangles.
                if (pArgs->dstX < pArgs->dstBoundsX)
                {
                    pArgs->srcWidth -= (pArgs->dstBoundsX - pArgs->dstX);
                    pArgs->srcX     += (pArgs->dstBoundsX - pArgs->dstX);
                    pArgs->dstWidth -= (pArgs->dstBoundsX - pArgs->dstX);
                    pArgs->dstX      =  pArgs->dstBoundsX;
                }

                if (pArgs->dstY < pArgs->dstBoundsY)
                {
                    pArgs->srcHeight -= (pArgs->dstBoundsY - pArgs->dstY);
                    pArgs->srcY      += (pArgs->dstBoundsY - pArgs->dstY);
                    pArgs->dstHeight -= (pArgs->dstBoundsY - pArgs->dstY);
                    pArgs->dstY       =  pArgs->dstBoundsY;
                }

                if (pArgs->dstX + pArgs->dstWidth > pArgs->dstBoundsX + pArgs->dstBoundsWidth)
                {
                    pArgs->srcWidth -= (pArgs->dstX + pArgs->dstWidth) - (pArgs->dstBoundsX + pArgs->dstBoundsWidth);
                    pArgs->dstWidth -= (pArgs->dstX + pArgs->dstWidth) - (pArgs->dstBoundsX + pArgs->dstBoundsWidth);
                }

                if (pArgs->dstY + pArgs->dstHeight > pArgs->dstBoundsY + pArgs->dstBoundsHeight)
                {
                    pArgs->srcHeight -= (pArgs->dstY + pArgs->dstHeight) - (pArgs->dstBoundsY + pArgs->dstBoundsHeight);
                    pArgs->dstHeight -= (pArgs->dstY + pArgs->dstHeight) - (pArgs->dstBoundsY + pArgs->dstBoundsHeight);
                }

                if (pArgs->dstWidth <= 0 || pArgs->dstHeight <= 0) {
                    return;
                }
            }
        }
    }

    if ((pArgs->options & EASY2D_IMAGE_DRAW_BACKGROUND) == 0 && pArgs->foregroundTint.r == 255 && pArgs->foregroundTint.g == 255 && pArgs->foregroundTint.b == 255)
    {
        // Fast path. No tint, no background.
        hSrcBitmap = pGDIImageData->hSrcBitmap;
    }
    else
    {
        // Slow path. We need to manually change the color values of the intermediate bitmap and use that as the source when drawing it.
        unsigned int* pSrcBitmapData = pGDIImageData->pSrcBitmapData;
        unsigned int* pDstBitmapData = pGDIImageData->pIntermediateBitmapData;
        for (unsigned int iRow = 0; iRow < pImage->height; ++iRow)
        {
            for (unsigned int iCol = 0; iCol < pImage->width; ++iCol)
            {
                unsigned int  srcTexel = *(pSrcBitmapData + (iRow * pImage->width) + iCol);
                unsigned int* dstTexel =  (pDstBitmapData + (iRow * pImage->width) + iCol);

                unsigned int srcTexelA = (srcTexel & 0xFF000000) >> 24;
                unsigned int srcTexelR = (unsigned int)(((srcTexel & 0x00FF0000) >> 16) * (pArgs->foregroundTint.r / 255.0f));
                unsigned int srcTexelG = (unsigned int)(((srcTexel & 0x0000FF00) >> 8)  * (pArgs->foregroundTint.g / 255.0f));
                unsigned int srcTexelB = (unsigned int)(((srcTexel & 0x000000FF) >> 0)  * (pArgs->foregroundTint.b / 255.0f));

                if (srcTexelR > 255) srcTexelR = 255;
                if (srcTexelG > 255) srcTexelG = 255;
                if (srcTexelB > 255) srcTexelB = 255;

                if ((pArgs->options & EASY2D_IMAGE_DRAW_BACKGROUND) != 0)
                {
                    srcTexelB += (unsigned int)(pArgs->backgroundColor.b * ((255 - srcTexelA) / 255.0f));
                    srcTexelG += (unsigned int)(pArgs->backgroundColor.g * ((255 - srcTexelA) / 255.0f));
                    srcTexelR += (unsigned int)(pArgs->backgroundColor.r * ((255 - srcTexelA) / 255.0f));
                    srcTexelA = 0xFF;
                }

                *dstTexel = (srcTexelR << 16) | (srcTexelG << 8) | (srcTexelB << 0) | (srcTexelA << 24);
            }
        }

        // Flush GDI to let it know we are finished with the bitmap object's data.
        GdiFlush();

        // If we have drawn the background manually we don't need to do an alpha blend.
        if ((pArgs->options & EASY2D_IMAGE_DRAW_BACKGROUND) != 0) {
            pArgs->options |= EASY2D_IMAGE_HINT_NO_ALPHA;
        }

        hSrcBitmap = pGDIImageData->hIntermediateBitmap;
    }


    if ((pArgs->options & EASY2D_IMAGE_DRAW_BOUNDS) != 0)
    {
        // The bounds is the area sitting around the outside of the destination rectangle.
        const float boundsLeft   = pArgs->dstBoundsX;
        const float boundsTop    = pArgs->dstBoundsY;
        const float boundsRight  = boundsLeft + pArgs->dstBoundsWidth;
        const float boundsBottom = boundsTop + pArgs->dstBoundsHeight;

        const float imageLeft   = pArgs->dstX;
        const float imageTop    = pArgs->dstY;
        const float imageRight  = imageLeft + pArgs->dstWidth;
        const float imageBottom = imageTop + pArgs->dstHeight;

        const int pPolyCounts[] = {4, 4, 4, 4};
        POINT pPoints[16];
        int polyCount = 0;
            
        POINT* pNextPoly = pPoints;

        // Left.
        if (boundsLeft < imageLeft)
        {
            pNextPoly[0].x = (LONG)(boundsLeft); pNextPoly[0].y = (LONG)(boundsTop);
            pNextPoly[1].x = (LONG)(boundsLeft); pNextPoly[1].y = (LONG)(boundsBottom);
            pNextPoly[2].x = (LONG)(imageLeft);  pNextPoly[2].y = (LONG)((imageBottom < boundsBottom) ? imageBottom : boundsBottom);
            pNextPoly[3].x = (LONG)(imageLeft);  pNextPoly[3].y = (LONG)((imageTop    > boundsTop)    ? imageTop    : boundsTop);

            pNextPoly += 4;
            polyCount += 1;
        }

        // Right.
        if (boundsRight > imageRight)
        {
            pNextPoly[0].x = (LONG)(boundsRight); pNextPoly[0].y = (LONG)(boundsBottom);
            pNextPoly[1].x = (LONG)(boundsRight); pNextPoly[1].y = (LONG)(boundsTop);
            pNextPoly[2].x = (LONG)(imageRight);  pNextPoly[2].y = (LONG)((imageTop    > boundsTop)    ? imageTop    : boundsTop);
            pNextPoly[3].x = (LONG)(imageRight);  pNextPoly[3].y = (LONG)((imageBottom < boundsBottom) ? imageBottom : boundsBottom);

            pNextPoly += 4;
            polyCount += 1;
        }

        // Top.
        if (boundsTop < imageTop)
        {
            pNextPoly[0].x = (LONG)(boundsRight);                                           pNextPoly[0].y = (LONG)(boundsTop);
            pNextPoly[1].x = (LONG)(boundsLeft);                                            pNextPoly[1].y = (LONG)(boundsTop);
            pNextPoly[2].x = (LONG)((imageLeft  > boundsLeft)  ? imageLeft  : boundsLeft);  pNextPoly[2].y = (LONG)(imageTop);
            pNextPoly[3].x = (LONG)((imageRight < boundsRight) ? imageRight : boundsRight); pNextPoly[3].y = (LONG)(imageTop);

            pNextPoly += 4;
            polyCount += 1;
        }

        // Bottom.
        if (boundsBottom > imageBottom)
        {
            pNextPoly[0].x = (LONG)(boundsLeft);                                            pNextPoly[0].y = (LONG)(boundsBottom);
            pNextPoly[1].x = (LONG)(boundsRight);                                           pNextPoly[1].y = (LONG)(boundsBottom);
            pNextPoly[2].x = (LONG)((imageRight < boundsRight) ? imageRight : boundsRight); pNextPoly[2].y = (LONG)(imageBottom);
            pNextPoly[3].x = (LONG)((imageLeft  > boundsLeft)  ? imageLeft  : boundsLeft);  pNextPoly[3].y = (LONG)(imageBottom);

            pNextPoly += 4;
            polyCount += 1;
        }

        if (polyCount > 0)
        {
            SelectObject(pGDISurfaceData->hDC, pGDISurfaceData->hStockNullPen);
            SelectObject(pGDISurfaceData->hDC, pGDISurfaceData->hStockDCBrush);
            SetDCBrushColor(pGDISurfaceData->hDC, RGB(pArgs->boundsColor.r, pArgs->boundsColor.g, pArgs->boundsColor.b));

            PolyPolygon(pGDISurfaceData->hDC, pPoints, pPolyCounts, polyCount);
        }
    }


    HGDIOBJ hPrevBitmap = SelectObject(pGDISurfaceData->hIntermediateDC, hSrcBitmap);
    if ((pArgs->options & EASY2D_IMAGE_HINT_NO_ALPHA) != 0)
    {
        StretchBlt(pGDISurfaceData->hDC, (int)pArgs->dstX, (int)pArgs->dstY, (int)pArgs->dstWidth, (int)pArgs->dstHeight, pGDISurfaceData->hIntermediateDC, (int)pArgs->srcX, (int)pArgs->srcY, (int)pArgs->srcWidth, (int)pArgs->srcHeight, SRCCOPY);
    }
    else
    {
        BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        AlphaBlend(pGDISurfaceData->hDC, (int)pArgs->dstX, (int)pArgs->dstY, (int)pArgs->dstWidth, (int)pArgs->dstHeight, pGDISurfaceData->hIntermediateDC, (int)pArgs->srcX, (int)pArgs->srcY, (int)pArgs->srcWidth, (int)pArgs->srcHeight, blend);
    }
    SelectObject(pGDISurfaceData->hIntermediateDC, hPrevBitmap);


    if (prevDC != 0) {
        RestoreDC(pGDISurfaceData->hDC, prevDC);
    }
}

void easy2d_set_clip_gdi(easy2d_surface* pSurface, float left, float top, float right, float bottom)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = easy2d_get_HDC(pSurface);

        SelectClipRgn(hDC, NULL);
        IntersectClipRect(hDC, (int)left, (int)top, (int)right, (int)bottom);
    }
}

void easy2d_get_clip_gdi(easy2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = easy2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        RECT rect;
        GetClipBox(easy2d_get_HDC(pSurface), &rect);

        if (pLeftOut != NULL) {
            *pLeftOut = (float)rect.left;
        }
        if (pTopOut != NULL) {
            *pTopOut = (float)rect.top;
        }
        if (pRightOut != NULL) {
            *pRightOut = (float)rect.right;
        }
        if (pBottomOut != NULL) {
            *pBottomOut = (float)rect.bottom;
        }
    }
}



bool easy2d_get_font_metrics_gdi(easy2d_font* pFont, easy2d_font_metrics* pMetricsOut)
{
    assert(pFont != NULL);
    assert(pMetricsOut != NULL);

    gdi_font_data* pGDIFontData = easy2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    *pMetricsOut = pGDIFontData->metrics;
    return true;
}

bool easy2d_get_glyph_metrics_gdi(easy2d_font* pFont, unsigned int utf32, easy2d_glyph_metrics* pGlyphMetrics)
{
    assert(pFont != NULL);
    assert(pGlyphMetrics != NULL);

    gdi_font_data* pGDIFontData = easy2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = easy2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    SelectObject(pGDIContextData->hDC, pGDIFontData->hFont);


    const MAT2 transform = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};        // <-- Identity matrix

    unsigned short utf16[2];
    int utf16Len = easy2d_utf32_to_utf16(utf32, utf16);

    WCHAR glyphIndices[2];

    GCP_RESULTSW glyphResults;
    ZeroMemory(&glyphResults, sizeof(glyphResults));
    glyphResults.lStructSize = sizeof(glyphResults);
    glyphResults.lpGlyphs = glyphIndices;
    glyphResults.nGlyphs  = 2;
    if (GetCharacterPlacementW(pGDIContextData->hDC, utf16, utf16Len, 0, &glyphResults, 0) != 0)
    {
        GLYPHMETRICS metrics;
        DWORD bitmapBufferSize = GetGlyphOutlineW(pGDIContextData->hDC, glyphIndices[0], GGO_NATIVE | GGO_GLYPH_INDEX, &metrics, 0, NULL, &transform);
        if (bitmapBufferSize != GDI_ERROR)
        {
            pGlyphMetrics->width    = metrics.gmBlackBoxX;
            pGlyphMetrics->height   = metrics.gmBlackBoxY;
            pGlyphMetrics->originX  = metrics.gmptGlyphOrigin.x;
            pGlyphMetrics->originY  = metrics.gmptGlyphOrigin.y;
            pGlyphMetrics->advanceX = metrics.gmCellIncX;
            pGlyphMetrics->advanceY = metrics.gmCellIncY;

            return true;
        }
    }
    
    return false;
}

bool easy2d_measure_string_gdi(easy2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut)
{
    assert(pFont != NULL);

    gdi_font_data* pGDIFontData = easy2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = easy2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    SelectObject(pGDIContextData->hDC, pGDIFontData->hFont);

    unsigned int textWLength;
    wchar_t* textW = easy2d_to_wchar_gdi(pFont->pContext, text, textSizeInBytes, &textWLength);
    if (textW != NULL)
    {
        SIZE sizeWin32;
        if (GetTextExtentPoint32W(pGDIContextData->hDC, textW, textWLength, &sizeWin32))
        {
            if (pWidthOut != NULL) {
                *pWidthOut = (float)sizeWin32.cx;
            }
            if (pHeightOut != NULL) {
                *pHeightOut = (float)sizeWin32.cy;
            }

            return true;
        }
    }

    return false;
}

bool easy2d_get_text_cursor_position_from_point_gdi(easy2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, unsigned int* pCharacterIndexOut)
{
    bool successful = false;

    assert(pFont != NULL);

    gdi_font_data* pGDIFontData = easy2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = easy2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    SelectObject(pGDIContextData->hDC, pGDIFontData->hFont);


    GCP_RESULTSW results;
    ZeroMemory(&results, sizeof(results));
    results.lStructSize = sizeof(results);
    results.nGlyphs     = (UINT)textSizeInBytes;

    unsigned int textWLength;
    wchar_t* textW = easy2d_to_wchar_gdi(pFont->pContext, text, textSizeInBytes, &textWLength);
    if (textW != NULL)
    {
        results.lpCaretPos = malloc(sizeof(int) * results.nGlyphs);
        if (results.lpCaretPos != NULL)
        {
            if (GetCharacterPlacementW(pGDIContextData->hDC, textW, results.nGlyphs, (int)maxWidth, &results, GCP_MAXEXTENT | GCP_USEKERNING) != 0)
            {
                unsigned int characterIndex = 0;
                float textCursorPosX = 0;

                for (unsigned int iChar = 0; iChar < results.nGlyphs; ++iChar)
                {
                    float charBoundsLeft  = (float)results.lpCaretPos[iChar];
                    float charBoundsRight = 0;
                    if (iChar < results.nGlyphs - 1) {
                        charBoundsRight = (float)results.lpCaretPos[iChar + 1];
                    } else {
                        charBoundsRight = maxWidth;
                    }

                    if (inputPosX >= charBoundsLeft && inputPosX <= charBoundsRight)
                    {
                        // The input position is somewhere on top of this character. If it's positioned on the left side of the character, set the output
                        // value to the character at iChar. Otherwise it should be set to the character at iChar + 1.
                        float charBoundsRightHalf = charBoundsLeft + ceilf(((charBoundsRight - charBoundsLeft) / 2.0f));
                        if (inputPosX <= charBoundsRightHalf) {
                            textCursorPosX = charBoundsLeft;
                            characterIndex = iChar;
                        } else {
                            textCursorPosX = charBoundsRight;
                            characterIndex = iChar + 1;
                        }
                    }
                }

                if (pTextCursorPosXOut) {
                    *pTextCursorPosXOut = textCursorPosX;
                }
                if (pCharacterIndexOut) {
                    *pCharacterIndexOut = characterIndex;
                }

                successful = true;
            }

            free(results.lpCaretPos);
        }
    }

    return successful;
}

bool easy2d_get_text_cursor_position_from_char_gdi(easy2d_font* pFont, const char* text, unsigned int characterIndex, float* pTextCursorPosXOut)
{
    bool successful = false;

    assert(pFont != NULL);

    gdi_font_data* pGDIFontData = easy2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = easy2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    SelectObject(pGDIContextData->hDC, pGDIFontData->hFont);


    GCP_RESULTSW results;
    ZeroMemory(&results, sizeof(results));
    results.lStructSize = sizeof(results);
    results.nGlyphs     = characterIndex + 1;

    unsigned int textWLength;
    wchar_t* textW = easy2d_to_wchar_gdi(pFont->pContext, text, (int)results.nGlyphs, &textWLength);
    if (textW != NULL)
    {
        results.lpCaretPos = malloc(sizeof(int) * results.nGlyphs);
        if (results.lpCaretPos != NULL)
        {
            if (GetCharacterPlacementW(pGDIContextData->hDC, textW, results.nGlyphs, 0, &results, GCP_USEKERNING) != 0)
            {
                if (pTextCursorPosXOut) {
                    *pTextCursorPosXOut = (float)results.lpCaretPos[characterIndex];
                }
                
                successful = true;
            }

            free(results.lpCaretPos);
        }
    }

    return successful;
}


wchar_t* easy2d_to_wchar_gdi(easy2d_context* pContext, const char* text, size_t textSizeInBytes, unsigned int* characterCountOut)
{
    if (pContext == NULL || text == NULL) {
        return NULL;
    }

    gdi_context_data* pGDIData = easy2d_get_context_extra_data(pContext);
    if (pGDIData == NULL) {
        return NULL;
    }


    int wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, NULL, 0);
    if (wcharCount == 0) {
        return NULL;
    }

    if (pGDIData->wcharBufferLength < (unsigned int)wcharCount + 1) {
        free(pGDIData->wcharBuffer);
        pGDIData->wcharBuffer       = malloc(sizeof(wchar_t) * (wcharCount + 1));
        pGDIData->wcharBufferLength = wcharCount + 1;
    }

    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, pGDIData->wcharBuffer, pGDIData->wcharBufferLength);
    if (wcharCount == 0) {
        return NULL;
    }


    if (characterCountOut != NULL) {
        *characterCountOut = wcharCount;
    }

    return pGDIData->wcharBuffer;
}

#endif  // GDI


/////////////////////////////////////////////////////////////////
//
// CAIRO 2D API
//
/////////////////////////////////////////////////////////////////
#ifndef EASY2D_NO_CAIRO

typedef struct
{
    cairo_surface_t* pCairoSurface;
    cairo_t* pCairoContext;

}cairo_surface_data;

bool easy2d_on_create_context_cairo(easy2d_context* pContext);
void easy2d_on_delete_context_cairo(easy2d_context* pContext);
bool easy2d_on_create_surface_cairo(easy2d_surface* pSurface, float width, float height);
void easy2d_on_delete_surface_cairo(easy2d_surface* pSurface);
void easy2d_begin_draw_cairo(easy2d_surface* pSurface);
void easy2d_end_draw_cairo(easy2d_surface* pSurface);
void easy2d_draw_rect_cairo(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color);


easy2d_context* easy2d_create_context_cairo()
{
    easy2d_drawing_callbacks callbacks;
    callbacks.on_create_context = easy2d_on_create_context_cairo;
    callbacks.on_delete_context = easy2d_on_delete_context_cairo;
    callbacks.on_create_surface = easy2d_on_create_surface_cairo;
    callbacks.on_delete_surface = easy2d_on_delete_surface_cairo;
    callbacks.begin_draw        = easy2d_begin_draw_cairo;
    callbacks.end_draw          = easy2d_end_draw_cairo;
    callbacks.draw_rect         = easy2d_draw_rect_cairo;

    return easy2d_create_context(callbacks, 0, sizeof(cairo_surface_data));
}

cairo_surface_t* easy2d_get_cairo_surface_t(easy2d_surface* pSurface)
{
    cairo_surface_data* pCairoData = easy2d_get_surface_extra_data(pSurface);
    if (pCairoData != NULL) {
        return pCairoData->pCairoSurface;
    }

    return NULL;
}

cairo_t* easy2d_get_cairo_t(easy2d_surface* pSurface)
{
    cairo_surface_data* pCairoData = easy2d_get_surface_extra_data(pSurface);
    if (pCairoData != NULL) {
        return pCairoData->pCairoContext;
    }

    return NULL;
}


bool easy2d_on_create_context_cairo(easy2d_context* pContext)
{
    assert(pContext != NULL);
    (void)pContext;

    return true;
}

void easy2d_on_delete_context_cairo(easy2d_context* pContext)
{
    assert(pContext != NULL);
    (void)pContext;
}

bool easy2d_on_create_surface_cairo(easy2d_surface* pSurface, float width, float height)
{
    assert(pSurface != NULL);

    cairo_surface_data* pCairoData = easy2d_get_surface_extra_data(pSurface);
    if (pCairoData == NULL) {
        return false;
    }

    pCairoData->pCairoSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)width, (int)height);
    if (pCairoData->pCairoSurface == NULL) {
        return false;
    }

    pCairoData->pCairoContext = cairo_create(pCairoData->pCairoSurface);
    if (pCairoData->pCairoContext == NULL) {
        cairo_surface_destroy(pCairoData->pCairoSurface);
        return false;
    }


    return true;
}

void easy2d_on_delete_surface_cairo(easy2d_surface* pSurface)
{
    assert(pSurface != NULL);

    cairo_surface_data* pCairoData = easy2d_get_surface_extra_data(pSurface);
    if (pCairoData != NULL)
    {
        cairo_destroy(pCairoData->pCairoContext);
        cairo_surface_destroy(pCairoData->pCairoSurface);
    }
}


void easy2d_begin_draw_cairo(easy2d_surface* pSurface)
{
    assert(pSurface != NULL);

    cairo_surface_data* pCairoData = easy2d_get_surface_extra_data(pSurface);
    if (pCairoData != NULL)
    {
    }
}

void easy2d_end_draw_cairo(easy2d_surface* pSurface)
{
    assert(pSurface != NULL);
    (void)pSurface;
}

void easy2d_draw_rect_cairo(easy2d_surface* pSurface, float left, float top, float right, float bottom, easy2d_color color)
{
    assert(pSurface != NULL);

    cairo_surface_data* pCairoData = easy2d_get_surface_extra_data(pSurface);
    if (pCairoData != NULL)
    {
        cairo_set_source_rgba(pCairoData->pCairoContext, color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
        cairo_rectangle(pCairoData->pCairoContext, left, top, (right - left), (bottom - top));
        cairo_fill(pCairoData->pCairoContext);
    }
}
#endif  // Cairo
#endif

/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/