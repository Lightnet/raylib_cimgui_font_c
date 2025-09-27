//===============================================
// use of rlgl and font stb_truetype lib
//===============================================
// due to conflict with the cimgui stb_truetype it has be sandboxed or isolated to use font lib.

#ifndef FONT_LOADER_H
#define FONT_LOADER_H

#define RLGL_STANDALONE
#include "rlgl.h"
#include "raylib.h"

// CustomFont struct for font data
typedef struct {
    unsigned int textureId;   // OpenGL texture ID from rlLoadTexture
    void* glyphs;            // Opaque pointer to hide stbtt_packedchar
    float size;              // Font size (pixel height)
    int atlasWidth, atlasHeight;  // Atlas dimensions
} CustomFont;

// Function declarations
CustomFont LoadCustomFont(const char *fileName, float fontSize);
void UnloadCustomFont(CustomFont font);
void DrawCustomText(CustomFont font, const char *text, float posX, float posY, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

#endif