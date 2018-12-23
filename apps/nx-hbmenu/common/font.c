#include "common.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __SWITCH__
#define FONT_FACES_MAX PlSharedFontType_Total
#else
#define FONT_FACES_MAX 2
#endif

static FT_Error s_font_libret=1, s_font_facesret[FONT_FACES_MAX];

static FT_Library s_font_library;
static FT_Face s_font_faces[FONT_FACES_MAX];
static FT_Face s_font_lastusedface;
static size_t s_font_faces_total = 0;

static bool FontSetType(u32 font)
{
    u32 i=0;
    u32 scale=0;
    FT_Error ret=0;

    switch(font)
    {
        case interuiregular14:
            scale = 4;
        break;

        case interuiregular18:
            scale = 5;
        break;

        case interuimedium20:
            scale = 6;
        break;

        case fontscale7:
            scale = 7;
        break;

        case interuimedium30:
            scale = 8;
        break;

        default:
            return false;
        break;
    }

    for (i=0; i<s_font_faces_total; i++) {
        ret = FT_Set_Char_Size(
                s_font_faces[i],        /* handle to face object           */
                0,                      /* char_width in 1/64th of points  */
                scale*64,               /* char_height in 1/64th of points */
                300,                    /* horizontal device resolution    */
                300);                   /* vertical device resolution      */

         if (ret) return false;
    }

    return true;
}

/*static inline const ffnt_page_t* FontGetPage(const ffnt_header_t* font, uint32_t page_id)
{
    //__builtin_printf("GetPage %u\n", (unsigned int)page_id);
    if (page_id >= font->npages)
        return NULL;
    ffnt_pageentry_t* ent = &((ffnt_pageentry_t*)(font+1))[page_id];
    if (ent->size == 0)
        return NULL;
    return (const ffnt_page_t*)((const uint8_t*)font + ent->offset);
}*/

static inline bool FontLoadGlyph(glyph_t* glyph, u32 font, uint32_t codepoint)
{
    FT_Face face;
    FT_Error ret=0;
    FT_GlyphSlot slot;
    FT_UInt glyph_index;
    FT_Bitmap* bitmap;
    u32 i=0;

    //__builtin_printf("LoadGlyph %u\n", (unsigned int)codepoint);
    /*const ffnt_page_t* page = FontGetPage(font, codepoint >> 8);
    if (!page)
        return false;

    codepoint &= 0xFF;
    uint32_t off = page->hdr.pos[codepoint];
    if (off == ~(uint32_t)0)
        return false;*/

    if (s_font_faces_total==0) return false;

    for (i=0; i<s_font_faces_total; i++) {
        face = s_font_faces[i];
        s_font_lastusedface = face;
        glyph_index = FT_Get_Char_Index(face, codepoint);
        if (glyph_index==0) continue;

        ret = FT_Load_Glyph(
                face,          /* handle to face object */
                glyph_index,   /* glyph index           */
                FT_LOAD_DEFAULT);

        if (ret==0)
        {
            ret = FT_Render_Glyph( face->glyph,             /* glyph slot  */
                                   FT_RENDER_MODE_NORMAL);  /* render mode */
        }

        if (ret) return false;

        break;
    }

    slot = face->glyph;
    bitmap = &slot->bitmap;

    //__builtin_printf("%c %u\n", (char)codepoint, (unsigned int)off);
    /*glyph->width   = page->hdr.widths[codepoint];
    glyph->height  = page->hdr.heights[codepoint];
    glyph->advance = page->hdr.advances[codepoint];
    glyph->posX    = page->hdr.posX[codepoint];
    glyph->posY    = page->hdr.posY[codepoint];
    glyph->data    = &page->data[off];*/

    glyph->width   = bitmap->width;
    glyph->height  = bitmap->rows;
    glyph->pitch   = bitmap->pitch;
    glyph->data    = bitmap->buffer;
    glyph->advance = slot->advance.x >> 6;
    glyph->posX    = slot->bitmap_left;
    glyph->posY    = slot->bitmap_top;
    return true;
}

static void DrawGlyph(uint32_t x, uint32_t y, color_t clr, const glyph_t* glyph)
{
    uint32_t i, j;
    const uint8_t* data = glyph->data;
    x += glyph->posX;
    y -= glyph->posY; //y += glyph->posY;
    //__builtin_printf("DrawGlyph %u %u %08X\n", (unsigned int)x, (unsigned int)y, (unsigned int)clr.abgr);
    for (j = 0; j < glyph->height; j ++)
    {
        for (i = 0; i < glyph->width; i ++)
        {
            clr.a = data[i];
            if (!clr.a) continue;
            DrawPixel(x+i, y+j, clr);
        }
        data+= glyph->pitch;
    }
}

static inline uint8_t DecodeByte(const char** ptr)
{
    uint8_t c = (uint8_t)**ptr;
    *ptr += 1;
    return c;
}

// UTF-8 code adapted from http://www.json.org/JSON_checker/utf8_decode.c

static inline int8_t DecodeUTF8Cont(const char** ptr)
{
    int c = DecodeByte(ptr);
    return ((c & 0xC0) == 0x80) ? (c & 0x3F) : -1;
}

static inline uint32_t DecodeUTF8(const char** ptr)
{
    uint32_t r;
    uint8_t c;
    int8_t c1, c2, c3;

    c = DecodeByte(ptr);
    if ((c & 0x80) == 0)
        return c;
    if ((c & 0xE0) == 0xC0)
    {
        c1 = DecodeUTF8Cont(ptr);
        if (c1 >= 0)
        {
            r = ((c & 0x1F) << 6) | c1;
            if (r >= 0x80)
                return r;
        }
    } else if ((c & 0xF0) == 0xE0)
    {
        c1 = DecodeUTF8Cont(ptr);
        if (c1 >= 0)
        {
            c2 = DecodeUTF8Cont(ptr);
            if (c2 >= 0)
            {
                r = ((c & 0x0F) << 12) | (c1 << 6) | c2;
                if (r >= 0x800 && (r < 0xD800 || r >= 0xE000))
                    return r;
            }
        }
    } else if ((c & 0xF8) == 0xF0)
    {
        c1 = DecodeUTF8Cont(ptr);
        if (c1 >= 0)
        {
            c2 = DecodeUTF8Cont(ptr);
            if (c2 >= 0)
            {
                c3 = DecodeUTF8Cont(ptr);
                if (c3 >= 0)
                {
                    r = ((c & 0x07) << 18) | (c1 << 12) | (c2 << 6) | c3;
                    if (r >= 0x10000 && r < 0x110000)
                        return r;
                }
            }
        }
    }
    return 0xFFFD;
}

static void DrawText_(u32 font, uint32_t x, uint32_t y, color_t clr, const char* text, uint32_t max_width, const char* end_text)
{
    //__builtin_printf("DrawText %u %u %08X %s\n", (unsigned int)x, (unsigned int)y, (unsigned int)clr.abgr, text);
    //y += font->baseline;
    uint32_t origX = x;
    if (s_font_faces_total==0) return;
    if (!FontSetType(font)) return;
    s_font_lastusedface = s_font_faces[0];

    while (*text)
    {
        if (max_width && x-origX >= max_width) {
            text = end_text;
            max_width = 0;
        }

        glyph_t glyph;
        uint32_t codepoint = DecodeUTF8(&text);

        if (codepoint == '\n')
        {
            if (max_width) {
                text = end_text;
                max_width = 0;
                continue;
            }

            x = origX;
            y += s_font_lastusedface->size->metrics.height / 64;
            continue;
        }

        if (!FontLoadGlyph(&glyph, font, codepoint))
        {
            if (!FontLoadGlyph(&glyph, font, '?'))
                continue;
        }

        DrawGlyph(x, y, clr, &glyph);
        x += glyph.advance;
    }
}

void DrawText(u32 font, uint32_t x, uint32_t y, color_t clr, const char* text)
{
    DrawText_(font, x, y, clr, text, 0, NULL);
}

void DrawTextTruncate(u32 font, uint32_t x, uint32_t y, color_t clr, const char* text, uint32_t max_width, const char* end_text)
{
    DrawText_(font, x, y, clr, text, max_width, end_text);
}

void GetTextDimensions(u32 font, const char* text, uint32_t* width_out, uint32_t* height_out)
{
    uint32_t x = 0;
    uint32_t width = 0, height = 0;
    if (s_font_faces_total==0) return;
    if (!FontSetType(font)) return;
    s_font_lastusedface = s_font_faces[0];

    while (*text)
    {
        glyph_t glyph;
        uint32_t codepoint = DecodeUTF8(&text);

        if (codepoint == '\n')
        {
            x = 0;
            height += s_font_lastusedface->size->metrics.height / 64;
            continue;
        }

        if (!FontLoadGlyph(&glyph, font, codepoint))
        {
            if (!FontLoadGlyph(&glyph, font, '?'))
                continue;
        }

        x += glyph.advance;

        if (x > width)
            width = x;
    }

    *width_out = width;
    *height_out = height;
}

bool fontInitialize(void)
{
    FT_Error ret=0;
    u32 i;

    for (i=0; i<FONT_FACES_MAX; i++) s_font_facesret[i] = 1;

    ret = FT_Init_FreeType(&s_font_library);
    s_font_libret = ret;
    if (s_font_libret) return false;

    #ifdef __SWITCH__
    PlFontData fonts[PlSharedFontType_Total];

    Result rc=0;
    rc = plGetSharedFont(textGetLanguageCode(), fonts, FONT_FACES_MAX, &s_font_faces_total);
    if (R_FAILED(rc)) return false;

    for (i=0; i<s_font_faces_total; i++) {
        ret = FT_New_Memory_Face( s_font_library,
                                  fonts[i].address,    /* first byte in memory */
                                  fonts[i].size,       /* size in bytes        */
                                  0,                   /* face_index           */
                                  &s_font_faces[i]);

        s_font_facesret[i] = ret;
        if (ret) return false;
    }
    #endif

    //These are loaded from "<original cwd>/fonts/<index>.ttf", these are user-supplied. Ideally these should be from plu SharedFont.
    #ifndef __SWITCH__
    char fontpath[PATH_MAX+1];

    for (i=0; i<FONT_FACES_MAX; i++) {
        memset(fontpath, 0, sizeof(fontpath));
        snprintf(fontpath, sizeof(fontpath)-1, "%s%sfonts%s%u.ttf", menuGetRootBasePath(), DIRECTORY_SEPARATOR, DIRECTORY_SEPARATOR, i);

        ret = FT_New_Face(        s_font_library,
                                  fontpath,
                                  0,
                                  &s_font_faces[i]);

        s_font_facesret[i] = ret;
        if (ret) return false;

        s_font_faces_total++;
    }
    #endif

    return true;
}

void fontExit()
{
    u32 i=0;

    for (i=0; i<s_font_faces_total; i++)
        if (s_font_facesret[i]==0) FT_Done_Face(s_font_faces[i]);

    if (s_font_libret==0) FT_Done_FreeType(s_font_library);
}

/*Automatically gives you the desired x-coordinate 
 *based on the string length and desired alignment
 *rY=reference point... where to align around
 *align='t','b','c' translates to (top,bottom,center)
 *'t' aligned, top of text aligns with rY, 
 *you get the rest....
 */
uint32_t GetTextYCoordinate(u32 font, uint32_t rY, const char* text, const char align) {
    uint32_t height_o,width;
    GetTextDimensions(font,text,&width,&height_o);
    uint32_t height = (uint32_t)height_o;
    uint32_t fC = (rY-height);

    switch(align){
        case 't':
        default:
            return rY;
        case 'c':
            return (rY+(height>>1));//>>1 is a bitwise shift for dividing by 2
        case 'b':
            if(fC<=0) return 0;
            else return fC;
    }
}

/*Automatically gives you the desired x-coordinate 
 *based on the string length and desired alignment
 *rX=reference point... where to align around
 *text=string you want to display
 *align='r','l','c' translates to (right,left,center)
 *'r' aligned, rX location = end of string, you get the rest...
 */
uint32_t GetTextXCoordinate(u32 font, uint32_t rX, const char* text, const char align) {
    uint32_t height,width_o;
    GetTextDimensions(font,text,&width_o,&height);
    uint32_t fC = (rX-width_o);

    switch(align){
        case 'r':
            if(fC<0) return 0;
            else return fC;
        case 'c':
            return (rX-(width_o>>1));//>>1 is a bitwise shift for dividing by 2
        case 'l':
        default:
            return rX;
    }
}
