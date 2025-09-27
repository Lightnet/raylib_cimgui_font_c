# raylib_cimgui_font_c

# License: MIT

# Information:
  Sample test for cimgui and font. As there was conflict on stb_truetype from imgui which is modified.

```
#include "stb_truetype.h"
```

```
#include "imstb_truetype.h"
```
  This from imgui.


# Overview of the Setup

This code is a basic integration test combining:

- GLFW for window creation, input handling, and OpenGL context management.
- raylib's rlgl.h (a lightweight OpenGL abstraction layer) for rendering 3D and 2D graphics without relying on raylib's full high-level API (e.g., no InitWindow or BeginDrawing).
- cimgui (C bindings for Dear ImGui) for overlaying a simple GUI.
- Custom font loading (via a presumed font_loader.h not shown here) for rendering text with stb_truetype.

The setup uses rlgl in a "standalone" mode (#define RLGL_STANDALONE), allowing direct control over OpenGL states while leveraging raylib's batching system for efficient drawing. This is useful for embedding raylib rendering into existing GLFW/OpenGL applications.Key goals of the code:

- Render a rotating 3D cube.
- Overlay 2D elements (a red quad and custom text).
- Add an ImGui window for interaction (e.g., a slider to control cube rotation).
- Demonstrate mixing 3D, 2D, and GUI rendering.

The code handles dynamic window resizing, VSync, depth testing, and blending. However, it has some redundancies (e.g., duplicate stb includes) and assumes font_loader.h provides LoadCustomFont and DrawCustomText implementations (likely using stb_truetype for font atlas packing and rlgl for texture rendering).

## Initialization Breakdown

1. GLFW Setup:
    - Initializes GLFW and creates a window with OpenGL 3.3 core profile.
    - Enables multisampling (4x), depth buffer (16-bit), and VSync.
    - Sets callbacks for errors, keys (e.g., ESC to close), and framebuffer resizing (updates viewport via rlViewport).
2. rlgl Initialization:
    - Calls rlLoadExtensions(glfwGetProcAddress) to load OpenGL functions.
    - rlglInit(screenWidth, screenHeight) sets up internal states, including vertex buffers for batching.
    - Sets clear color with rlClearColor.
    - Enables depth testing with rlEnableDepthTest for 3D.
3. Camera Setup:
    - 3D: A perspective camera looking at the origin from (5,5,5).
    - 2D: An orthographic camera with top-left origin (common in raylib), centered on the screen.
4. Custom Font:
    - Loads a TTF font (e.g., "Kenney Pixel.ttf") at a specific size. This likely involves stb_truetype to bake glyphs into a texture atlas, which is uploaded via rlgl (e.g., rlLoadTexture).
5. ImGui Setup:
    - Creates ImGui context and initializes backends for GLFW and OpenGL3.
    - Applies dark style and enables keyboard navigation.
6. Other States:
    - Prints current front-face winding (defaults to GL_CCW in OpenGL).
    - Disables backface culling in some commented sections, but it's not active.

## Main Loop Structure and Draw Order

The loop follows a strict sequence to ensure correct layering: background (3D), midground (2D), foreground (ImGui). This is critical because rlgl uses batching—drawing commands are queued and flushed with rlDrawRenderBatchActive(). Mixing modes requires explicit state changes (e.g., projection matrices, depth testing).

Here's the step-by-step flow in the main loop, focusing on draw order and rlgl usage:

1. Update Logic:
    - Compute auto-rotation for the cube (30°/sec) unless the ImGui slider is active (igIsItemActive()).
    - Poll GLFW events.
    - Get current framebuffer size (handles resizing).
2. Clear Buffers:
    - rlClearScreenBuffers() clears color and depth buffers. This must happen early to reset the frame. The color is set earlier to light yellow.
3. Start ImGui Frame:
    - ImGui_ImplOpenGL3_NewFrame(), ImGui_ImplGlfw_NewFrame(), igNewFrame().
    - Build UI: A window with text and a slider for cube rotation. Note: This records draw commands but doesn't render yet.
4. Record ImGui (but Defer Rendering):
    - igRender() prepares draw data. Actual rendering happens later to layer ImGui on top.
5. 3D Rendering Block:
    
    - Enable depth test: rlEnableDepthTest().
    - Set perspective projection: MatrixPerspective(...) and rlSetMatrixProjection(proj).
    - Compute view matrix: MatrixLookAt(...).
    - Compute model matrix: Rotation (Y-axis) + translation.
    - Combine model-view: MatrixMultiply(model, view) and rlSetMatrixModelview(modelView). This bypasses rlgl's matrix stack for direct control.
    - Draw the cube: DrawCube(...) (a high-level raylib function that queues vertices/colors to rlgl's batch). Uses gray color.
    - Flush batch: rlDrawRenderBatchActive(). This sends queued 3D draws to GPU.
    
    Draw Order Note: 3D is drawn first, with depth testing to handle occlusion within the scene. No backface culling disable here, so default GL_CCW winding applies (cube faces are likely defined CCW).
6. 2D Rendering Block:
    
    - Disable depth test: rlDisableDepthTest() (prevents 2D from being occluded by 3D depth).
    - Enable blending: glEnable(GL_BLEND) and glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) for transparency (though not used in this quad/text).
    - Set orthographic projection: MatrixOrtho(0, width, height, 0, -1, 1) and rlSetMatrixProjection(proj2). Note: Y-axis is flipped (height to 0), making y increase downward (raylib convention for 2D).
    - Set identity model-view: rlSetMatrixModelview(MatrixIdentity()).
    - Draw red quad using rlBegin(RL_QUADS) / rlEnd():
        - The "working" vertex order is TL -> BL -> BR -> TR (clockwise in y-down coords).
        - Important Winding Issue: With flipped Y in projection, effective winding reverses. OpenGL expects CCW for front faces by default. The flipped projection makes clockwise windings appear CCW to the GPU. The non-working order (TL -> TR -> BR -> BL, clockwise without flip consideration) would be culled if culling were enabled. Here, culling isn't explicitly disabled for 2D, but the code relies on the flip to make clockwise intuitive for y-down.
    - Draw custom text: DrawCustomText(...) (assumed to use rlgl for textured quads with font atlas).
    - Flush batch: rlDrawRenderBatchActive().
    - Disable blending: rlDisableColorBlend().
    
    Draw Order Note: 2D draws after 3D, so it overlays without depth interference. Use clockwise winding for 2D quads when Y is flipped.
7. ImGui Rendering:
    - ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData()). This draws the UI on top using its own shaders/states. ImGui handles its own 2D ortho projection and blending.
8. Swap Buffers:
    - glfwSwapBuffers(window) presents the frame.

## Correct Way to Handle Draw Order (for Documentation)

When using rlgl standalone with GLFW (as here), draw order is managed by sequencing calls in the loop. rlgl batches draws efficiently but doesn't enforce layers—you must handle states manually. Here's a recommended approach based on this setup:General Principles

- Clear Early: Always rlClearScreenBuffers() at the start to reset color/depth.
- Batch Flushing: Call rlDrawRenderBatchActive() after each major section (e.g., 3D, 2D) to ensure previous draws are committed before state changes.
- State Isolation: Explicitly enable/disable features like depth test (rlEnableDepthTest() / rlDisableDepthTest()) and blending when switching modes. Use rlSetMatrixProjection and rlSetMatrixModelview to set matrices directly.
- Layering: Draw from back to front:
    1. 3D (with depth for internal occlusion).
    2. 2D overlays (without depth to avoid clipping).
    3. GUI (e.g., ImGui) last, as it assumes ortho and blends on top.
- Winding Order:
    - For 3D: Use CCW (counter-clockwise) for front faces (OpenGL default).
    - For 2D with flipped Y (raylib-style ortho where y increases down): Use clockwise vertex order (e.g., TL -> BL -> BR -> TR) to match effective CCW after flip. If culling is enabled, mismatched winding causes invisibility.
    - Query with glGetIntegerv(GL_FRONT_FACE, &frontFace) if unsure; adjust with glFrontFace(GL_CW) if needed.
- Resizing: Update viewport with rlViewport(0, 0, width, height) in the resize callback. Recompute projections in the loop using current size.
- Performance: Minimize state changes. Group similar draws (e.g., all 3D before 2D).
- Debugging Tips:
    - Print states (e.g., front face) to verify.
    - Disable culling temporarily (rlDisableBackfaceCulling()) to test visibility.
    - If mixing with ImGui, defer its render to last to avoid overwriting.

Example Draw Order Template (Pseudocode)

```text
while (!shouldClose) {
    // Update logic...

    rlClearScreenBuffers();  // Clear color + depth

    // 3D: Enable depth, set perspective proj + modelview, draw, flush
    rlEnableDepthTest();
    rlSetMatrixProjection(perspectiveMatrix);
    rlSetMatrixModelview(modelViewMatrix);
    Draw3DStuff();
    rlDrawRenderBatchActive();

    // 2D: Disable depth, set ortho proj (flipped Y), identity modelview, draw (clockwise winding), flush
    rlDisableDepthTest();
    glEnable(GL_BLEND); glBlendFunc(...);
    rlSetMatrixProjection(orthoMatrix);
    rlSetMatrixModelview(identityMatrix);
    Draw2DStuff();
    rlDrawRenderBatchActive();
    glDisable(GL_BLEND);

    // GUI: Render ImGui or other overlays last

    SwapBuffers();
}
```

This setup ensures 3D is at the back, 2D in the middle, and GUI on top. Adapt for your needs, e.g., add sorting for transparent 2D elements. If using full raylib, prefer BeginMode3D / BeginMode2D for simpler state management.

# Overview of font_loader.h

This header file (font_loader.h) provides a custom font loading and rendering system using the STB Truetype library (stb_truetype.h). It is designed to work with raylib's low-level OpenGL abstraction (rlgl.h) for rendering textured glyphs in a 2D or 3D context. The implementation is "sandboxed" (isolated) to avoid conflicts with other libraries like cimgui, which may include their own versions of STB Truetype.Key features:

- Loads TrueType Font (TTF) files and bakes glyphs (ASCII 32-126) into a texture atlas.
- Supports oversampling for smoother rendering.
- Renders text as textured quads using rlgl's drawing commands, with blending for alpha transparency.
- Manages memory and GPU resources properly (loading/unloading).

The code defines an internal CustomFont struct (aliased as InternalCustomFont) to store the texture ID, glyph data, size, and atlas dimensions. Public functions include LoadCustomFont, UnloadCustomFont, and DrawCustomText.This is useful in setups where raylib's high-level font API isn't available (e.g., standalone rlgl with GLFW), allowing custom font rendering without dependencies on raylib's full font system.Dependencies and Defines

- #define STB_TRUETYPE_IMPLEMENTATION: Enables the implementation code in stb_truetype.h.
- Includes: stb_truetype.h, stdlib.h, stdio.h.
- Assumes raylib utilities like LoadFileData, UnloadFileData, rlLoadTexture, rlTextureParameters, and rlgl drawing functions (e.g., rlBegin, rlVertex2f) are available.
- Glyph range: Limited to printable ASCII (32-126) for simplicity; extend range.num_chars if needed for more characters.

## Struct Definition

c
```c
typedef struct {
    unsigned int textureId;      // OpenGL texture ID for the atlas.
    stbtt_packedchar glyphs[95]; // Packed glyph data for ASCII 32-126.
    float size;                  // Requested font size in pixels.
    int atlasWidth, atlasHeight; // Dimensions of the texture atlas (fixed at 512x512).
} InternalCustomFont;
```

- This is typedef'd to CustomFont (assumed declared in the header).
- glyphs: Array of stbtt_packedchar structs, each containing UV coords, offsets, and advances for a character.
- Fixed atlas size (512x512): May need increasing for larger fonts or more glyphs to avoid packing failures.

## Function: LoadCustomFont

c
```c
CustomFont LoadCustomFont(const char *fileName, float fontSize);
```

Description: Loads a TTF font file, packs glyphs into a grayscale bitmap, converts to RGBA, uploads as an OpenGL texture via rlgl, and stores glyph metrics.Parameters:

- fileName: Path to the TTF file (e.g., "Kenney Pixel.ttf").
- fontSize: Pixel height for glyphs (e.g., 18.0f).

Steps:

1. Load file data using LoadFileData (raylib utility; reads binary file into memory).
2. Initialize STB font info with stbtt_InitFont.
3. Allocate a grayscale bitmap (512x512) for the atlas.
4. Set up packing context with stbtt_PackBegin, enabling 2x oversampling for anti-aliasing.
5. Define a packing range for ASCII 32-126 and pack with stbtt_PackFontRanges.
6. Convert grayscale to RGBA (white RGB with alpha from grayscale intensity) for OpenGL compatibility.
7. Upload texture with rlLoadTexture (uncompressed RGBA8 format, 1 mipmap).
8. Free temporary buffers and file data.
9. Store glyphs (dynamically allocated in the range) in the font struct.
10. Set bilinear filtering on the texture for smooth scaling.

Error Handling: Prints errors and returns a zero-initialized font if loading/init/packing fails. Check font.textureId != 0 after calling.Notes:

- Oversampling (2x) improves quality but increases atlas usage.
- Memory for range.chardata_for_range is malloc'd and must be freed in UnloadCustomFont.
- Atlas is grayscale-to-RGBA for white text; modify RGB channels for colored fonts.
- If packing fails (e.g., atlas too small), stbtt_PackFontRanges returns 0—add checks to resize atlas dynamically.

Function: UnloadCustomFont

c
```c
void UnloadCustomFont(CustomFont font);
```

Description: Releases GPU texture and glyph memory.Parameters:

- font: The loaded CustomFont to unload.

Steps:

1. Unload texture with rlUnloadTexture.
2. Free the glyph array (allocated in LoadCustomFont).

Notes: Call this before program exit to avoid leaks. Safe to call on invalid fonts (e.g., textureId == 0).Function: DrawCustomText

c
```c
void DrawCustomText(CustomFont font, const char *text, float posX, float posY, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
```

Description: Renders the text string as a series of textured quads using the font's atlas. Supports positioning and color (including alpha for transparency).Parameters:

- font: Loaded CustomFont.
- text: Null-terminated string to draw (ASCII only).
- posX, posY: Screen position (top-left of the text baseline, in pixels). Assumes raylib's 2D coord system (Y increasing downward).
- r, g, b, a: Color components (0-255). Alpha blends with background.

Steps:

1. Bind the font texture with rlSetTexture.
2. Enable alpha blending: rlEnableColorBlend() and rlSetBlendMode(RL_BLEND_ALPHA) (src alpha, one-minus-src-alpha).
3. Push matrix stack with rlPushMatrix (saves current transform; though translation is commented out).
4. Initialize cursor x = posX (horizontal advance starts here).
5. For each character in text:
    - If printable ASCII, get index (text[i] - 32).
    - Compute quad (positioned billboard) with stbtt_GetPackedQuad: Advances x and posY (though posY is baseline-fixed here).
    - Draw quad in RL_QUADS mode:
        - Set color with rlColor4ub.
        - Normal for lighting (flat, Z-facing).
        - Texcoords and vertices: TL -> BL -> BR -> TR (clockwise, suitable for raylib's flipped Y in 2D ortho).
6. Pop matrix with rlPopMatrix.
7. Unbind texture with rlSetTexture(0).

Commented Alternative:

- The blocked-out version uses rlTranslatef(posX, posY, 0.0f) for positioning and starts x = 0.0f relative to translation.
- Vertex order: TL -> TR -> BR -> BL (counter-clockwise without flip consideration).
- Current version positions via absolute rlVertex2f coords (no translation), with clockwise winding. This matches the main code's 2D quad example, accounting for Y-flip in ortho projection (makes clockwise effective CCW to GPU).

Draw Order and Integration Notes:

- Call this in a 2D rendering block (after setting ortho projection and identity modelview, as in the main setup).
- Assumes depth test is disabled (for overlay).
- Blending must be enabled globally or per-call.
- Kerning/advances are handled by STB (via quad.xadvance).
- No word wrapping or multiline support—render line-by-line if needed.
- Performance: Batches quads; flush with rlDrawRenderBatchActive() after all text draws in a frame.
- Winding: Clockwise vertices work due to Y-flipped projection (common in raylib 2D). If issues, disable culling or reverse order.

Potential Issues:

- If posY advances unexpectedly, it's due to baseline alignment in stbtt_GetPackedQuad—adjust for descenders (e.g., 'g').
- Non-ASCII chars are skipped.
- For colored text, modify RGBA in atlas or per-quad.

## Usage Example (from Main Code)

c
```c
CustomFont font = LoadCustomFont("Kenney Pixel.ttf", 18.0f);
...
DrawCustomText(font, "Custom Font Test", 100.0f, 100.0f, 255, 0, 255, 255);  // Purple text
...
UnloadCustomFont(font);
```

This integrates seamlessly with rlgl's batching system, allowing custom fonts in minimal setups. For production, add error checks, support more codepoints, or dynamic atlas sizing.


# Credits:
- raylib github
- cimgui github
- https://kenney.nl/assets/kenney-fonts
- https://github.com/nothings/stb
- Grok AI with help of rlgl.h for low level abstraction.