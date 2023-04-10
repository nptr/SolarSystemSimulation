#include <GLFW/glfw3.h>

bool OnInit(GLFWwindow* window);

void OnExit(GLFWwindow* window);

/**
 * Called when the user want to close the window
*/
void OnWindowClosing(GLFWwindow* window);

/**
 * Called when the window resizes
*/
void OnResizeViewport(GLFWwindow* window, int width, int height);

/**
 * Called when a key was pressed.
 * It saves the key for eventual evaluation by ProcessInput()
 */
void OnKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);

/**
 * Called when the cursor position changed. Triggers also on hidden cursors!
 * The function updates the camera based on the cursor movement.
 */
void OnCursorPosChanged(GLFWwindow* window, double x, double y);

/**
 * Called when a mouse button was pressed on ESC (see ProcessInput())
 */
void OnMouseButtonClick(GLFWwindow* window, int button, int action, int mod);

/**
 * Handles the pressed keys (multiple presses+handling possible)
 * @param window The current window.
 * @param dt Amount of simulation ticks that passed since
 * the last invocation. This value is framerate independent.
 */
void ProcessInput(GLFWwindow* window, double dt);

/**
 * Routine responsible to calculate & update all objects state. => MVP matrix and other properties.
 * Called before rendering, once per frame, but with a time reference as parameter to be CPU speed independent.
 * @param dt Number of elapsed simulation ticks. <SIMULATION_FREQ> ticks / s.
 */
void Update(double dt);

/**
 * Routines responsible for rendering. Called once per frame.
 */
void Display(double dt);