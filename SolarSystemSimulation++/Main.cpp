#include "gl_core_3_3.h" // OpenGL binding
#include <GLFW/glfw3.h>	 // Window handling

#include "Logger.h"
#include "SolarSystemSimulation.h"

const float FRAMERATE = 60.0f;
const float FRAMETIME = 1.0f / FRAMERATE;

int main(void)
{
    Logger& log = Logger::GetInstance();

    if (!glfwInit())
    {
        log.Log(LogLevel::ERROR, "Failed to initialize GLFW.\r\nThis should never happen...");
        return -1;
    }

    // Create a windowed mode window and a OpenGL context:
    // OGL 3.3: for ARB_explicit_attrib_location (thats 2010)
    // OGL 3.2: for ARB_seamless_cube_map
    // OGL 3.1: not needed, but later for ARB_draw_instanced
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SRGB_CAPABLE, true);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Solar System Simulation - A Project of Jakob K.", NULL, NULL);
    if (!window)
    {
        log.Log(LogLevel::ERROR, "Failed to create a window :(\r\nThis should never happen...");
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize the OpenGL Loading Library - MUST BE AFTER glfwMakeContextCurrent()
    if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
    {
        log.Log(LogLevel::ERROR, "Failed to retrieve function pointers for opengl");
        return -1;
    }

    log.Log(LogLevel::INFO, "Renderer: \n%s\n\n", (const char*)glGetString(GL_RENDERER));

    if(!OnInit(window))
    {
        return -1;
    }

    // Setup Window callbacks
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, OnKeyPressed);
    glfwSetMouseButtonCallback(window, OnMouseButtonClick);
    glfwSetCursorPosCallback(window, OnCursorPosChanged);
    glfwSetWindowSizeCallback(window, OnResizeViewport);
    glfwSetWindowCloseCallback(window, OnExit);

    double simOldTime = glfwGetTime();
    double simNewTime;
    double simTickDiff;

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        simNewTime = glfwGetTime();
        simTickDiff = (simNewTime - simOldTime) / FRAMETIME;
        simOldTime = simNewTime;

        // We update as often as possible and also interpolate the simulationtime to get that extra bit of smoothness.
        // Rendering is also done as often as possible
        ProcessInput(window, simTickDiff);
        Update(simTickDiff);
        Display(simTickDiff);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    OnExit(window);
    glfwTerminate();

#ifdef _DEBUG
    getchar();
#endif
    return 0;
}