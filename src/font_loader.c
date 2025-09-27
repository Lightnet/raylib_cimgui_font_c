// due to conflict with the cimgui stb_truetype it has be sandboxed to use font
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "font_loader.h"
#include <stdlib.h>
#include <stdio.h>

// Define CustomFont internally with stbtt_packedchar
typedef struct {
    unsigned int textureId;
    stbtt_packedchar glyphs[95];  // ASCII 32-126
    float size;
    int atlasWidth, atlasHeight;
} InternalCustomFont;

CustomFont LoadCustomFont(const char *fileName, float fontSize) {
    CustomFont font = {0};
    font.size = fontSize;

    // Load TTF file data
    unsigned int fileSize;
    unsigned char *fileData = LoadFileData(fileName, &fileSize);
    if (fileData == NULL) {
        printf("Failed to load font file: %s\n", fileName);
        return font;
    }

    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, fileData, 0)) {
        UnloadFileData(fileData);
        printf("Failed to initialize font\n");
        return font;
    }

    // Atlas setup
    font.atlasWidth = 512;
    font.atlasHeight = 512;
    unsigned char *bitmap = (unsigned char *)calloc(font.atlasWidth * font.atlasHeight, sizeof(unsigned char));

    stbtt_pack_context packContext;
    stbtt_PackBegin(&packContext, bitmap, font.atlasWidth, font.atlasHeight, 0, 1, NULL);
    stbtt_PackSetOversampling(&packContext, 2, 2);

    // Pack ASCII chars 32-126
    stbtt_pack_range range = {0};
    range.font_size = fontSize;
    range.first_unicode_codepoint_in_range = 32;
    range.num_chars = 95;
    range.chardata_for_range = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * 95);
    stbtt_PackFontRanges(&packContext, fileData, 0, &range, 1);

    stbtt_PackEnd(&packContext);

    // Convert grayscale bitmap to RGBA
    unsigned char *rgbaData = (unsigned char *)malloc(font.atlasWidth * font.atlasHeight * 4);
    for (int i = 0; i < font.atlasWidth * font.atlasHeight; i++) {
        rgbaData[i * 4 + 0] = 255;  // R
        rgbaData[i * 4 + 1] = 255;  // G
        rgbaData[i * 4 + 2] = 255;  // B
        rgbaData[i * 4 + 3] = bitmap[i];  // A
    }
    free(bitmap);

    // Upload to GPU
    font.textureId = rlLoadTexture(rgbaData, font.atlasWidth, font.atlasHeight, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    free(rgbaData);
    UnloadFileData(fileData);

    // Store glyphs
    font.glyphs = range.chardata_for_range;

    // Set texture filter
    rlTextureParameters(font.textureId, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_BILINEAR);
    rlTextureParameters(font.textureId, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_BILINEAR);

    return font;
}

void UnloadCustomFont(CustomFont font) {
    rlUnloadTexture(font.textureId);
    free(font.glyphs);  // Free allocated glyph data
}


void DrawCustomText(CustomFont font, const char *text, float posX, float posY, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    stbtt_packedchar* glyphs = (stbtt_packedchar*)font.glyphs;

    rlSetTexture(font.textureId);

    rlEnableColorBlend();
    rlSetBlendMode(RL_BLEND_ALPHA);

    rlPushMatrix();
    // rlTranslatef(posX, posY, 0.0f);
    // rlTranslatef(0.0f, 0.0f, 0.0f);

    // float x = 0.0f;
    float x = posX;// position 2d
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] >= 32 && text[i] < 127) {
            int index = text[i] - 32;
            stbtt_aligned_quad quad;
            stbtt_GetPackedQuad(glyphs, font.atlasWidth, font.atlasHeight, index, &x, &posY, &quad, 1);

            rlBegin(RL_QUADS);
            rlColor4ub(r, g, b, a);
            rlNormal3f(0.0f, 0.0f, 1.0f);

            rlTexCoord2f(quad.s0, quad.t0); rlVertex2f(quad.x0, quad.y0);  // Top-left
            rlTexCoord2f(quad.s0, quad.t1); rlVertex2f(quad.x0, quad.y1);  // Bottom-left
            rlTexCoord2f(quad.s1, quad.t1); rlVertex2f(quad.x1, quad.y1);  // Bottom-right
            rlTexCoord2f(quad.s1, quad.t0); rlVertex2f(quad.x1, quad.y0);  // Top-right
            rlEnd();
        }
    }
    // rlTranslatef(100.0f, 0.0f, 0.0f);

    rlPopMatrix();
    rlSetTexture(0);
}


/*
void DrawCustomText(CustomFont font, const char *text, float posX, float posY, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    stbtt_packedchar* glyphs = (stbtt_packedchar*)font.glyphs;

    rlSetTexture(font.textureId);

    rlEnableColorBlend();
    rlSetBlendMode(RL_BLEND_ALPHA);

    rlPushMatrix();
    rlTranslatef(posX, posY, 0.0f);

    float x = 0.0f;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] >= 32 && text[i] < 127) {
            int index = text[i] - 32;
            stbtt_aligned_quad quad;
            stbtt_GetPackedQuad(glyphs, font.atlasWidth, font.atlasHeight, index, &x, &posY, &quad, 1);

            rlBegin(RL_QUADS);
            rlColor4ub(r, g, b, a);
            rlNormal3f(0.0f, 0.0f, 1.0f);

            rlTexCoord2f(quad.s0, quad.t0); rlVertex2f(quad.x0, quad.y0);
            rlTexCoord2f(quad.s1, quad.t0); rlVertex2f(quad.x1, quad.y0);
            rlTexCoord2f(quad.s1, quad.t1); rlVertex2f(quad.x1, quad.y1);
            rlTexCoord2f(quad.s0, quad.t1); rlVertex2f(quad.x0, quad.y1);
            rlEnd();
        }
    }

    rlPopMatrix();
    rlSetTexture(0);
}
*/