//===============================================
// Base simple setup test for raylib and cimgui.
// single file test
//===============================================
// #ifndef STB_RECT_PACK_IMPLEMENTATION
// #define STB_RECT_PACK_IMPLEMENTATION
// #include "stb_rect_pack.h"
// #endif

// #ifndef STB_TRUETYPE_IMPLEMENTATION
// #define STB_TRUETYPE_IMPLEMENTATION
// #include "stb_truetype.h"
// #endif

// #include "stb_rect_pack.h"

// #include "imstb_rectpack.h"  // Provides stbrp_* declarations // from imgui
// #include "imstb_truetype.h"  // Provides stbtt_* declarations (packing uses stbrp_*) // imgui

#include "cimgui.h"
#include "cimgui_impl.h"

// #define RLGL_IMPLEMENTATION
#define RLGL_STANDALONE
#include "raylib.h"
#include "rlgl.h"
#define RAYMATH_STATIC_INLINE
#include "raymath.h"
#include "font_loader.h"
#include <GLFW/glfw3.h>
// #include "utils.h"  // Optional: for LoadFileData/UnloadFileData if you want raylib utils

#include <stdlib.h>
#include <stdio.h>              // Required for: printf()
#include <math.h>               // For fmodf

#define igGetIO igGetIO_Nil

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void ErrorCallback(int error, const char *description);
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);  // For resize

int main() {
    int screenWidth = 800;
    int screenHeight = 450;
    const char *glsl_version = "#version 130";

    // Initialize GLFW
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "rlgl + ImGui + 3D Cube", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwSetWindowPos(window, 200, 200);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);  // Handle resize

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable VSync for smoothness

    // Load OpenGL 3.3 supported extensions
    rlLoadExtensions(glfwGetProcAddress);

    // Initialize OpenGL context (states and resources)
    rlglInit(screenWidth, screenHeight);


    // Set clear color (do this after rlglInit)
    rlClearColor(245, 245, 200, 255);  // Light yellow background

    // Enable depth test for 3D
    rlEnableDepthTest();

    // Camera setup
    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point (cube)
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector
    camera.fovy = 45.0f;                                // Field of view Y

    // Set up 2D camera
    Camera2D camera2d = { 0 };
    camera2d.target = (Vector2){ 0.0f, 0.0f }; // Center of the camera
    camera2d.offset = (Vector2){ 400.0f, 300.0f }; // Screen center (800/2, 600/2)
    camera2d.rotation = 0.0f;
    camera2d.zoom = 1.0f; // 1:1 scale for screen coordinates


    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };        // Cube at center
    float rotation = 0.0f;  // For animation (updated by slider or auto)

    // Load custom font
    // CustomFont font = LoadCustomFont("Kenney Pixel.ttf", 12.0f);  // Replace with actual font path
    CustomFont font = LoadCustomFont("Kenney Pixel.ttf", 18.0f);  // Replace with actual font path

    if (font.textureId == 0) {
        printf("Error: Font failed to load properly\n");
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Setup ImGui
    igCreateContext(NULL);
    ImGuiIO *ioptr = igGetIO();
    ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGuiStyle* style = igGetStyle();
    // Optional: float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    // ImGuiStyle_ScaleAllSizes(style, main_scale);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    igStyleColorsDark(NULL);

    GLint frontFace;
    glGetIntegerv(GL_FRONT_FACE, &frontFace);
    printf("FrontFace: %s\n", frontFace == GL_CW ? "GL_CW" : "GL_CCW");

    // Disable backface culling to ensure the square is visible
    // rlDisableBackfaceCulling();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float time = (float)glfwGetTime();
        // Auto-rotate if not actively using slider (simple fallback; slider overrides)
        if (igIsItemActive() == false) {  // Check if slider is not being dragged
            rotation = fmodf(time * 30.0f, 360.0f);  // 30 degrees per second
        }

        glfwPollEvents();

        // Get current size (for dynamic support)
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

        // Clear early (color + depth for 3D)
        rlClearScreenBuffers();

        // // ImGui frame start
        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // igNewFrame();

        // // Build ImGui UI
        // igBegin("Hello, world!", NULL, 0);
        // igText("This is some useful text.");
        // igText("3D Cube should now rotate below!");
        // igText("Current Rotation: %.1f degrees", rotation);
        // if (igSliderFloat("Cube Y Rotation", &rotation, 0.0f, 360.0f, "%.0f degrees", 0)) {
        //     // Slider changed - rotation updates immediately
        // }
        // igEnd();
        // // End ImGui frame (record lists)
        // igRender();

        // // Enable depth test for 3D
        // rlEnableDepthTest();
        // // 3D Rendering Setup
        // float aspect = (float)screenWidth / (float)screenHeight;
        // Matrix proj = MatrixPerspective(camera.fovy * DEG2RAD, aspect, 0.1f, 1000.0f);  // Perspective projection
        // rlSetMatrixProjection(proj);

        // // Compute view matrix from camera
        // Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);

        // // Compute model matrix: rotation * translation
        // Matrix rot = MatrixRotateY(rotation * DEG2RAD);  // Rotate around Y
        // Matrix trans = MatrixTranslate(cubePosition.x, cubePosition.y, cubePosition.z);
        // Matrix model = MatrixMultiply(rot, trans);

        // // Full model-view matrix (apply model to view)
        // Matrix modelView = MatrixMultiply(model, view);

        // // Set the full model-view directly (bypass stack)
        // rlSetMatrixModelview(modelView);

        // // Draw the cube (no push/pop or mult needed)
        // // CustomDrawCube((Vector3){0.0f, 0.0f, 0.0f});  // At local origin, with model applied above
        // DrawCube((Vector3){0.0f, 0.0f, 0.0f}, 1.0f, 1.0f, 1.0f, GRAY);

        // rlDrawRenderBatchActive();  // Flush the batch

        // 2D rendering setup
        rlDisableDepthTest();  // Disable depth test for 2D
        // rlDisableBackfaceCulling();
        glEnable(GL_BLEND);  // Enable blending (already done in your code)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Set orthographic projection
        Matrix proj = MatrixOrtho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
        rlSetMatrixProjection(proj);  // Allowed since it's in the main loop, not font rendering

        // Set identity modelview
        Matrix modelView = MatrixIdentity();
        rlSetMatrixModelview(modelView);  // Allowed since it's in the main loop

        // Draw quad
        // rlBegin(RL_QUADS);
        //     rlColor4ub(255, 0, 0, 255);  // Red color
        //     rlVertex2f(100.0f, 100.0f);  // Top-left
        //     rlVertex2f(300.0f, 100.0f);  // Top-right
        //     rlVertex2f(300.0f, 200.0f);  // Bottom-right
        //     rlVertex2f(100.0f, 200.0f);  // Bottom-left
        // rlEnd();

        rlBegin(RL_QUADS);
            rlColor4ub(255, 0, 0, 255);  // Red color
            rlVertex2f(100.0f, 100.0f);  // Top-left
            rlVertex2f(100.0f, 200.0f);  // Bottom-left
            rlVertex2f(300.0f, 200.0f);  // Bottom-right
            rlVertex2f(300.0f, 100.0f);  // Top-right
        rlEnd();


        // Draw custom text
        // DrawCustomText(font, "Custom Font Test", 10.0f, 10.0f, 100, 100, 100, 100);
        DrawCustomText(font, "Custom Font Test", 100.0f, 100.0f, 255, 0, 255, 255);  // Solid white

        rlDrawRenderBatchActive();  // Flush the batch
        rlDisableColorBlend();  // Disable blending

        // Reset state for ImGui
        // glUseProgram(0);

        // Render ImGui (on top, 2D)
        // ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);
    rlglClose();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definitions
//----------------------------------------------------------------------------------

static void ErrorCallback(int error, const char *description) {
    fprintf(stderr, "%s", description);
}

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

// Resize callback: Update viewport
static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    rlViewport(0, 0, width, height);
}