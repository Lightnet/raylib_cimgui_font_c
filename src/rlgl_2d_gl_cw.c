//  clockwise vertex order
// 


#include "rlgl.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "raymath.h"

static void ErrorCallback(int error, const char *description);
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

int main() {
    int screenWidth = 800;
    int screenHeight = 450;

    // Initialize GLFW
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "rlgl 2D Square", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwSetWindowPos(window, 200, 200);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetErrorCallback(ErrorCallback);

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    // Load OpenGL extensions
    rlLoadExtensions(glfwGetProcAddress);

    // Initialize rlgl
    rlglInit(screenWidth, screenHeight);

    // Set clear color
    rlClearColor(245, 245, 200, 255); // Light yellow background

    // Set clockwise winding as front-facing
    glFrontFace(GL_CW); // Make clockwise polygons front-facing

    GLint frontFace;
    glGetIntegerv(GL_FRONT_FACE, &frontFace);
    printf("FrontFace: %s\n", frontFace == GL_CW ? "GL_CW" : "GL_CCW");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Get current framebuffer size
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

        // Clear the screen (color buffer)
        rlClearScreenBuffers();

        // Ensure blending is enabled for proper alpha handling
        rlEnableColorBlend();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Set orthographic projection (2D screen space: top-left origin)
        Matrix proj = MatrixOrtho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
        rlSetMatrixProjection(proj);

        // Set modelview to identity for direct screen-space drawing
        Matrix modelView = MatrixIdentity();
        rlSetMatrixModelview(modelView);

        // Unbind texture for solid color
        rlSetTexture(0);

        // Draw a 2D square (quad) at position (100, 100) with size 200x200
        rlBegin(RL_QUADS);
            rlColor4ub(255, 0, 0, 255); // Red color
            // Original clockwise winding order
            rlVertex2f(100.0f, 100.0f); // Top-left
            rlVertex2f(300.0f, 100.0f); // Top-right
            rlVertex2f(300.0f, 300.0f); // Bottom-right
            rlVertex2f(100.0f, 300.0f); // Bottom-left
        rlEnd();

        // Flush the render batch
        rlDrawRenderBatchActive();

        // Disable blending after drawing
        rlDisableColorBlend();

        // Debugging: Print OpenGL error if any
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            printf("OpenGL Error: %d\n", err);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    rlglClose();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// Module Functions Definitions
static void ErrorCallback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    rlViewport(0, 0, width, height);
}