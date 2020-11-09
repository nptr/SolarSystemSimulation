#include "gl_core_3_3.h"					// OpenGL binding
#include <glfw/glfw3.h>						// Window handling

#include <glm/glm.hpp>						// Math
#include <glm/gtx/transform.hpp>			// 3D transforms
#include <glm/gtx/matrix_transform_2d.hpp>	// 2D transforms

#include "jge/Mesh.h"
#include "jge/Model.h"
#include "jge/RenderPipeline.h"
#include "jge/Scene.h"
#include "jge/Camera.h"
#include "jge/LightSource.h"
#include "jge/Measurement.h"
#include "jge/Texture.h"
#include "jge/Util.h"

#include "PlanetInfo.h"
#include "PlanetData.h"
#include "PlanetMovementSystem.h"
#include "GpuInfo.h"

#include "imgui\imgui.h"
#include "imgui_impl_glfw_gl3.h"

#ifdef _DEBUG
	#include "jge/Memory.h"
#endif

using namespace jge;
using namespace glm;

#define NEAR_PLANE 0.15f
#define FAR_PLANE 250.0f

GLFWwindow* window;
bool isFullscreen = false;
int windowWidth = 1280;
int windowHeight = 800;
int prevWidth, prevHeight;
int prevX, prevY;

// Keyboard Map
bool keyStates[512];

// Basic Stuff
Scene* scene;
Camera* camera;
RenderPipeline* pipeline;

// Simulation parameters (in Hz)
const float SIMULATION_FREQ = 60.0f;
const float SIMULATION_DELTA = 1.0f / SIMULATION_FREQ;
float simSpeeds[] =
{
	0.0f,
    0.03f / SIMULATION_FREQ,
	0.1f / SIMULATION_FREQ,
	1.0f / SIMULATION_FREQ,
	5.0f / SIMULATION_FREQ,
	10.0f / SIMULATION_FREQ,
	50.0f / SIMULATION_FREQ,
	200.0f / SIMULATION_FREQ,
	400.0f / SIMULATION_FREQ,
	800.0f / SIMULATION_FREQ,
};
double simulationTime;
float simulationTick = simSpeeds[2];

// The Scene Composition
LightSource* sunLight;
ShaderProgram lightingShader;
ShaderProgram shadowShader;
ShaderProgram skyboxShader;
ShaderProgram blur2DShader;
ShaderProgram glowmapShader;
ShaderProgram blendShader;
ShaderProgram textShader;
ShaderProgram starShader;

// Meshes
Mesh highPolySphere;
Mesh mediumPolySphere;
Mesh lowPolySphere;
Mesh ringMesh;
Mesh circleMesh;
Mesh starMesh;

// Models (Meshes + Texture)
Model bodies[12];
Model skybox;
Model saturnrings;
Model orbits[8];
Model stars;

FrameCounter fpsCounter;

// Forward declarations
void Display();
void DisplayUi();
void InitGraphics();
void InitData();
void Update(double simTime);
void DoPlanetSelection(glm::vec3 rayOrigin, glm::vec3 rayDirection);

GLuint LoadShader(GLenum type, const char* path);
GLuint LoadTexture(const char* file);
GLuint LoadTexture(const char* file, GLuint wrapMode, bool srgbInternal);
void LoadStarCatalog(Mesh& m);

/**
 * Called when the windowsize changed.
 * It resizes OpenGLs viewport and adjusts the cameras perspective
 */
void OnResizeViewport(GLFWwindow* wnd, int width, int height)
{
	// 0 can occur on Win+D!
	if (width <= 0 || height <= 0)
		return;

	windowWidth = width;
	windowHeight = height;

	glViewport(0, 0, width, height);
	camera->SetPerspective(width / (float)height, NEAR_PLANE, FAR_PLANE);
	
	pipeline->SetSize(width, height);
}


/**
 * Called when a key was pressed.
 * It saves the key for eventual evaluation by ProcessInput()
 */
void OnKeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// on fn + [..] key is negative
	if (key < 0)
		return;

	// Change simulation tick with 0 - 9 Keys
	if (action == GLFW_PRESS && key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
	{
		simulationTick = simSpeeds[key - GLFW_KEY_0];
		return;
	}

	if (action == GLFW_PRESS || action == GLFW_REPEAT)
		keyStates[key] = true;
	else keyStates[key] = false;
}


/**
 * Called when the cursor position changed. Triggers also on hidden cursors!
 * The function updates the camera based on the cursor movement.
 */
void OnCursorPosChangedCallback(GLFWwindow* window, double x, double y)
{
    // This variable is a hack to prevent the initial camera jump when the cursor isn't caught yet
    static bool caught_cursor = false;
    // This variable is a hack to stop glfwSetCursorPos from triggering OnCursorPosChangedCallback() recursively.
    // Maybe using GLFW_CURSOR_DISABLED is the correct approach?
    static bool just_warped = false;

	if (just_warped) {
		just_warped = false;
		return;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		int dx = (int)x - windowWidth / 2;
		int dy = (int)y - windowHeight / 2;

		if (caught_cursor) {
			camera->RotateYaw((float)dx);
            camera->RotatePitch((float)-dy);
		}

		just_warped = true;
        caught_cursor = true;
		glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
	}
    else {
        caught_cursor = false;
    }
}


/**
 * Called when a mouse button was pressed
 * on ESC (see ProcessInput())
 */
void OnMouseButtonClick(GLFWwindow* window, int button, int action, int mod)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        double nx = 2.0 * x / windowWidth - 1.0;
        double ny = 1.0 - 2.0 * y / windowHeight;

        glm::vec4 rayClip = glm::vec4(nx, ny, -1.0f, 1.0f);

        glm::mat4 invProj = glm::inverse(camera->GetProjectionMatrix());
        glm::vec4 rayView = invProj * rayClip;
        rayView.z = -1.0f;
        rayView.w = 0.0f;

        glm::mat4 invView = glm::inverse(camera->GetViewMatrix());
        glm::vec4 rayWorld4 = invView * rayView;
        glm::vec3 rayWorld = glm::normalize(glm::vec3(rayWorld4.x, rayWorld4.y, rayWorld4.z));

        DoPlanetSelection(camera->GetPosition(), rayWorld);
    }

	if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glfwSetCursorPosCallback(window, OnCursorPosChangedCallback);
	}
}


/**
 * Called when the window is closing
 * Here is the place where we (should) release all acquired resources
 */
void OnWindowClosing(GLFWwindow* window)
{
	delete pipeline;
	delete scene;
	delete camera;
	delete sunLight;
}


/**
 * Handles the pressed keys (multiple presses+handling possible)
 * @param fracLastInput Amount of simulation ticks that passed since
 * the last invocation. This value is framerate independent.
 */
void ProcessInput(double fracLastInput)
{
    float incr = fracLastInput * 1.0f;

	if (keyStates['w'] || keyStates['W'])
	{
		camera->Move(incr);
	}
	if (keyStates['a'] || keyStates['A'])
	{
		camera->Strafe(incr);
	}
	if (keyStates['s'] || keyStates['S'])
	{
		camera->Move(-incr);
	}
	if (keyStates['d'] || keyStates['D'])
	{
		camera->Strafe(-incr);
	}
	if (keyStates['t'] || keyStates['T'])
	{
		scene->EnableTriangleSorting(!scene->IsTriangleSortingEnabled());
	}
	if (keyStates[GLFW_KEY_F11])
	{
		keyStates[GLFW_KEY_F11] = false;
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if (!isFullscreen)
		{
			isFullscreen = true;
			prevWidth = windowWidth;
			prevHeight = windowHeight;
			glfwGetWindowPos(window, &prevX, &prevY);

			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			isFullscreen = false;
			glfwSetWindowMonitor(window, NULL, prevX, prevY, prevWidth, prevHeight, 0);
		}
	}
	if (keyStates[GLFW_KEY_ESCAPE])
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetCursorPosCallback(window, 0);
	}
}


/**
 * Entry Point: Manages OpenGL context creation, window creation, gameloop
 */
int main(void)
{
	if (!glfwInit())
	{
		MessageBox(NULL, "Failed to initialize GLFW.\r\nThis should never happen...", "Much Uh-oh :/", MB_OK | MB_ICONERROR);
		return -1;
	}
	
	// Create a windowed mode window and a OpenGL context:
	// OGL 3.3: for ARB_explicit_attrib_location (thats 2010)
	// OGL 3.2: for ARB_seamless_cube_map
	// OGL 3.1: not needed, but later for ARB_draw_instanced
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SRGB_CAPABLE, true);
	window = glfwCreateWindow(windowWidth, windowHeight, "Solar System Simulation - A Project of Jakob K.", NULL, NULL);
	if (!window)
	{
		MessageBox(NULL, "Failed to create a window :(\r\nThis should never happen...", "Much Uh-oh :/", MB_OK | MB_ICONERROR);
		glfwTerminate();
		return -1;
	}


	// Make the window's context current
	glfwMakeContextCurrent(window);
	
	// Initialize the OpenGL Loading Library - MUST BE AFTER glfwMakeContextCurrent()
	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		MessageBox(NULL, "Failed to retrieve function pointers for opengl", "Uh-oh :/", MB_OK | MB_ICONERROR);
		return -1;
	}

	printf("Renderer: \n%s\n\n", (const char*)glGetString(GL_RENDERER));

	// Make the default framebuffer gamma aware
	glEnable(GL_FRAMEBUFFER_SRGB);

	// Setup Camera
	camera = new Camera();
	camera->SetPerspective(windowWidth / (float)windowHeight, NEAR_PLANE, FAR_PLANE);

	// View from above
	//camera->SetYaw(-0.366519004);
	//camera->SetPitch(-0.841248274);
	//camera->SetPosition(glm::vec3(-70.1717758, 101.857643, 23.3185806));
	
	// View from Sun
	camera->SetYaw(-0.4153f);
	camera->SetPitch(-0.4084f);
	camera->SetPosition(glm::vec3(-22.4035f, 12.2775f, 7.1834f));
    camera->SetMoveSpeed(0.25f);

	ImGui_ImplGlfwGL3_Init(window, true);

	try
	{
		InitGraphics();
		InitData();
	}
	catch (const std::runtime_error& ex)
	{
		MessageBox(NULL, ex.what(), "SSS - Runtime Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	catch (const std::exception& ex)
	{
		MessageBox(NULL, ex.what(), "SSS - Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	// Setup Window callbacks
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, OnKeyPressedCallback);
	glfwSetMouseButtonCallback(window, OnMouseButtonClick);
	glfwSetCursorPosCallback(window, OnCursorPosChangedCallback);
	glfwSetWindowSizeCallback(window, OnResizeViewport);
	glfwSetWindowCloseCallback(window, OnWindowClosing);

	double simOldTime = glfwGetTime();
	double simNewTime;
	double simTickDiff;
	double inputOldTime = glfwGetTime();

	simulationTime = 0.0f;

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		ImGui_ImplGlfwGL3_NewFrame();

		// The simulationtime gets incremented every delta-t (0.01667s) = 60Hz by the simulation speed (simulationTick).
		simNewTime = glfwGetTime();
        simTickDiff = (simNewTime - simOldTime) / SIMULATION_DELTA;
		simOldTime = simNewTime;
		simulationTime += simTickDiff * simulationTick;
		
		// We update as often as possible and also interpolate the simulationtime to get that extra bit of smoothness.
		// Rendering is also done as often as possible
        ProcessInput(simTickDiff);
		Update(simulationTime);
		Display();
		DisplayUi();

		// Display frame
		glfwSwapBuffers(window);
		fpsCounter.Tick();

		glfwPollEvents();
	}

	ImGui_ImplGlfwGL3_Shutdown();

	glfwTerminate();

#ifdef _DEBUG
	printMemTrackResult();
	getchar();
#endif
	return 0;
}


/**
 * Sets up the rendering pipeline. Called once on start.
 */
void InitGraphics()
{
	Stopwatch sw;
	sw.Start();

	// Load shaders and create shaderprograms
	GLuint defaultVShader = LoadShader(GL_VERTEX_SHADER, "shader\\master.vert");
	GLuint defaultFShader = LoadShader(GL_FRAGMENT_SHADER, "shader\\masterOptimised.frag");
	lightingShader.Create(defaultVShader, defaultFShader);
	lightingShader.RegisterUniform("useTexture[1]");	// uniform doesn't count as active, so register it manually?
	lightingShader.RegisterUniform("texTransform[1]");	// uniform doesn't count as active, so register it manually?

	GLuint shadowVShader = LoadShader(GL_VERTEX_SHADER, "shader\\vsm.vert");
	GLuint shadowFShader = LoadShader(GL_FRAGMENT_SHADER, "shader\\vsm.frag");
	shadowShader.Create(shadowVShader, shadowFShader);

	GLuint skyboxVShader = LoadShader(GL_VERTEX_SHADER, "shader\\skybox.vert");
	GLuint skyboxFShader = LoadShader(GL_FRAGMENT_SHADER, "shader\\skybox.frag");
	skyboxShader.Create(skyboxVShader, skyboxFShader);

	GLuint textVShader = LoadShader(GL_VERTEX_SHADER, "shader\\text.vert");
	GLuint textFShader = LoadShader(GL_FRAGMENT_SHADER, "shader\\text.frag");
	textShader.Create(textVShader, textFShader);

	GLuint basicVShader = LoadShader(GL_VERTEX_SHADER, "shader\\basic.vert");
	GLuint solidFShader = LoadShader(GL_FRAGMENT_SHADER, "shader\\solidColor.frag");
	glowmapShader.Create(basicVShader, solidFShader);

	GLuint basic2DVShader = LoadShader(GL_VERTEX_SHADER, "shader\\basic2D.vert");
	GLuint blur2DFShader = LoadShader(GL_FRAGMENT_SHADER, "shader\\gaussBlur9x1.frag");
	blur2DShader.Create(basic2DVShader, blur2DFShader);

	GLuint blendFShader = LoadShader(GL_FRAGMENT_SHADER, "shader\\blend.frag");
	blendShader.Create(basic2DVShader, blendFShader);

	GLuint starVShader = LoadShader(GL_VERTEX_SHADER, "shader\\stars.vert");
	GLuint starFShader = LoadShader(GL_FRAGMENT_SHADER, "shader\\stars.frag");
	starShader.Create(starVShader, starFShader);

	sw.Stop();
	double shaderTiming = sw.GetElapsedTime();
	printf("Loaded shaders in %3.3f seconds.\r\n", shaderTiming);

	// Setup Light
	lightingShader.UseProgram();
    sunLight = LightSource::CreatePointLight(glm::vec3(0.0f, 0.0f, 0.0f));
    //sunLight = LightSource::CreateDirectionalLight(glm::vec3(0.0f, 0.0f, -1.0f));
    //sunLight = LightSource::CreateSpotLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), 90.0f, 0.5);
	sunLight->SetColor(LightComponent::DIFFUSE, 1.0f, 1.0f, 0.90f);
	sunLight->SetColor(LightComponent::SPECULAR, 0.8f, 0.8f, 0.55f);
	sunLight->SetColor(LightComponent::AMBIENT, 0.001f, 0.001f, 0.001f);
	sunLight->SetAttenuation(1.0f, 0.02f, 0.0f);

	// The scene holds and manages all objects to render
	scene = new Scene();
	scene->SetCamera(camera);
	scene->AddLight(sunLight);

	// The renderpipeline renders the objects
	pipeline = new RenderPipeline(scene);
	pipeline->SetSize(windowWidth, windowHeight);
	pipeline->SupplyShaders(
		&shadowShader,
		&lightingShader,
		&blur2DShader,
		&blendShader,
		&skyboxShader,
		&glowmapShader,
		&starShader);
	pipeline->Build();
}


/**
 * Loads all models and populates the scene to be rendered. Called
 * once on program start.
 */
void InitData()
{
	Stopwatch sw;
	sw.Start();

	// Load and set the cube texture for the skybox
	GLuint skyboxTexture = Texture::LoadCubemap(
		"texture\\skybox\\posx.png",
		"texture\\skybox\\negx.png",
		"texture\\skybox\\posy.png",
		"texture\\skybox\\negy.png",
		"texture\\skybox\\posz.png",
		"texture\\skybox\\negz.png");
	scene->SetSkyboxTexture(skyboxTexture);

	// Load the planet textures
	GLuint sunTex =			LoadTexture("texture\\sun\\sun.png");
	GLuint sunTex2 =		LoadTexture("texture\\sun\\clouds.png");
	GLuint mercuryTex =		LoadTexture("texture\\mercury\\mercury.png");
	GLuint mercuryNormal =	LoadTexture("texture\\mercury\\normal.png", GL_CLAMP_TO_EDGE, false);
	GLuint venusTex =		LoadTexture("texture\\venus\\venus.png");
	GLuint earthTex =		LoadTexture("texture\\earth\\color.png");
	GLuint earthTex2 =		LoadTexture("texture\\earth\\clouds.png");
	GLuint earthSpecTex =	LoadTexture("texture\\earth\\specular.png");
	GLuint earthNormal =	LoadTexture("texture\\earth\\normals.png", GL_CLAMP_TO_EDGE, false);
	GLuint marsTex =		LoadTexture("texture\\mars\\mars.png");
	GLuint marsNormal =		LoadTexture("texture\\mars\\normal.png", GL_CLAMP_TO_EDGE, false);
	GLuint jupiterTex =		LoadTexture("texture\\jupiter\\jupiter.png");
	GLuint saturnTex =		LoadTexture("texture\\other\\saturn.png");
	GLuint saturnRingTex =	LoadTexture("texture\\other\\saturnrings.png");
	GLuint uranusTex =		LoadTexture("texture\\other\\uranus.png");
	GLuint neptuneTex =		LoadTexture("texture\\other\\neptune.png");

	// Load the moon textures
	GLuint moonTex =	LoadTexture("texture\\moon\\moon.png");
	GLuint moonNormal = LoadTexture("texture\\moon\\normal.png", GL_CLAMP_TO_EDGE, false);
	GLuint ioTex =		LoadTexture("texture\\jupiter\\jupiter_io.png");
	GLuint europaTex =	LoadTexture("texture\\jupiter\\jupiter_europa.png");

	sw.Stop();
	double textureTiming = sw.GetElapsedTime();
	printf("Loaded textures in %3.3f seconds.\r\n", textureTiming);
	sw.Start();

	// Load sphere models in different detail levels
	highPolySphere.FromObjectFile("models\\sphereHighPoly.obj", true);
	mediumPolySphere.FromObjectFile("models\\sphereMediumPoly.obj", true);
	lowPolySphere.FromObjectFile("models\\sphereLowPoly.obj", true);

	// Other models
	ringMesh.FromObjectFile("models\\saturnrings.obj");

	// Stars
	LoadStarCatalog(starMesh);

	sw.Stop();
	double modelTiming = sw.GetElapsedTime();
	printf("Loaded models in %3.3f seconds.\r\n", modelTiming);

	stars.SetMeshes(&starMesh);
	stars.SetAffectedByLighting(false);
	stars.SetShadowCasting(false);
	stars.SetTextureUsage(false);
	stars.SetSortTriangles(false);
	
	// Angles empirically determinded to match the skybox.
	// Make the starsphere bigger than the rest of the scene so the z-test
	// always sees the stars as furthest
	float starangleX = 3.574f;
	float starangleY = 0.060f;
	float starangleZ = 8.928f;
	mat4 one = glm::rotate(starangleY, vec3(0, -1, 0));
	mat4 two = glm::rotate(starangleZ, vec3(0, 0, -1));
	mat4 three = glm::rotate(starangleX, vec3(-1, 0, 0));
	mat4 scale = glm::scale(vec3(150.0, 150.0, 150.0));
	stars.modelMatrix = one * two * three * scale;
	stars.SetShader(&starShader);

	circleMesh.CreateCircle();
	orbits[0].SetMeshes(&circleMesh);
	orbits[0].SetAffectedByLighting(false);
	orbits[0].SetShadowCasting(false);
	orbits[0].SetTextureUsage(false);
	orbits[1] = orbits[2] = orbits[3] = orbits[4] = orbits[5] = orbits[6] = orbits[7] = orbits[0];
	orbits[0].modelMatrix = glm::scale(glm::rotate(mercuryInfo.orbitInclination, vec3(0, 0, 1)), glm::vec3(mercuryInfo.distanceToParent, 0, mercuryInfo.distanceToParent));
	orbits[1].modelMatrix = glm::scale(glm::rotate(venusInfo.orbitInclination, vec3(0, 0, 1)), glm::vec3(venusInfo.distanceToParent, 0, venusInfo.distanceToParent));
	orbits[2].modelMatrix = glm::scale(glm::rotate(earthInfo.orbitInclination, vec3(0, 0, 1)), glm::vec3(earthInfo.distanceToParent, 0, earthInfo.distanceToParent));
	orbits[3].modelMatrix = glm::scale(glm::rotate(marsInfo.orbitInclination, vec3(0, 0, 1)), glm::vec3(marsInfo.distanceToParent, 0, marsInfo.distanceToParent));
	orbits[4].modelMatrix = glm::scale(glm::rotate(jupiterInfo.orbitInclination, vec3(0, 0, 1)), glm::vec3(jupiterInfo.distanceToParent, 0, jupiterInfo.distanceToParent));
	orbits[5].modelMatrix = glm::scale(glm::rotate(saturnInfo.orbitInclination, vec3(0, 0, 1)), glm::vec3(saturnInfo.distanceToParent, 0, saturnInfo.distanceToParent));
	orbits[6].modelMatrix = glm::scale(glm::rotate(uranusInfo.orbitInclination, vec3(0, 0, 1)), glm::vec3(uranusInfo.distanceToParent, 0, uranusInfo.distanceToParent));
	orbits[7].modelMatrix = glm::scale(glm::rotate(neptuneInfo.orbitInclination, vec3(0, 0, 1)), glm::vec3(neptuneInfo.distanceToParent, 0, neptuneInfo.distanceToParent));
	float ringAlpha = 0.1f;
	orbits[0].SetColor(vec4(0.3f, 0.3f, 0.3f, ringAlpha));
	orbits[1].SetColor(vec4(0.3f, 0.3f, 0.3f, ringAlpha));
	orbits[2].SetColor(vec4(0.3f, 0.3f, 0.3f, ringAlpha));
	orbits[3].SetColor(vec4(0.3f, 0.3f, 0.3f, ringAlpha));
	orbits[4].SetColor(vec4(0.3f, 0.3f, 0.3f, ringAlpha));
	orbits[5].SetColor(vec4(0.3f, 0.3f, 0.3f, ringAlpha));
	orbits[6].SetColor(vec4(0.3f, 0.3f, 0.3f, ringAlpha));
	orbits[7].SetColor(vec4(0.3f, 0.3f, 0.3f, ringAlpha));

	// Sun is not affected by lighting (light source is inside the sun)
	// static modelmatrix in case i am removing the animation in Update()
    bodies[bSun].modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sunInfo.planetSize, sunInfo.planetSize, sunInfo.planetSize));
    bodies[bSun].SetMeshes(&highPolySphere);
    bodies[bSun].SetTexture(sunTex, 0);
    bodies[bSun].SetTexture(sunTex2, 1);
    bodies[bSun].SetAffectedByLighting(false);
    bodies[bSun].SetColor(vec4(1.0f, 0.7f, 0.2f, 1.0f));
    bodies[bSun].SetGlowEffect(true);

	// Planets
    bodies[bMercury].SetMeshes(&mediumPolySphere);
    bodies[bMercury].SetTexture(mercuryTex);
    bodies[bMercury].SetNormalMapUsage(0, true);
    bodies[bMercury].SetNormalMap(mercuryNormal);
    bodies[bVenus].SetMeshes(&mediumPolySphere);
    bodies[bVenus].SetTexture(venusTex);
    bodies[bEarth].SetMeshes(&highPolySphere, &mediumPolySphere);
    bodies[bEarth].SetTexture(earthTex, 0);
    bodies[bEarth].SetTexture(earthTex2, 1);
    bodies[bEarth].SetNormalMapUsage(0, true);
    bodies[bEarth].SetNormalMap(earthNormal);
    bodies[bEarth].SetSpecularMapUsage(0, true);
    bodies[bEarth].SetSpecularMap(earthSpecTex);
    bodies[bEarth].SetBlendMode(BlendMode::SCREEN);
	bodies[bMars].SetMeshes(&mediumPolySphere, &lowPolySphere);
    bodies[bMars].SetTexture(marsTex);
    bodies[bMars].SetNormalMapUsage(0, true);
    bodies[bMars].SetNormalMap(marsNormal);
    bodies[bJupiter].SetMeshes(&highPolySphere, &mediumPolySphere);
    bodies[bJupiter].SetTexture(jupiterTex);
    bodies[bSaturn].SetMeshes(&highPolySphere, &mediumPolySphere);
    bodies[bSaturn].SetTexture(saturnTex);
	saturnrings.SetMeshes(&ringMesh);
	saturnrings.SetTexture(saturnRingTex);
    saturnrings.SetShadowCasting(false);
    saturnrings.SetTransparent(true);
    //saturnrings.SetSortTriangles(true);
    bodies[bUranus].SetMeshes(&mediumPolySphere, &lowPolySphere);
    bodies[bUranus].SetTexture(uranusTex);
    bodies[bNeptune].SetMeshes(&mediumPolySphere, &lowPolySphere);
    bodies[bNeptune].SetTexture(neptuneTex);

	// Moons
    bodies[bMoon].SetMeshes(&mediumPolySphere);
    bodies[bMoon].SetTexture(moonTex);
    bodies[bMoon].SetNormalMapUsage(0, true);
    bodies[bMoon].SetNormalMap(moonNormal);
    bodies[bIo].SetMeshes(&lowPolySphere);
    bodies[bIo].SetTexture(ioTex);
    bodies[bEuropa].SetMeshes(&lowPolySphere);
    bodies[bEuropa].SetTexture(europaTex);

	// Add the models to the scene
	scene->AddModel(&bodies[bMercury]);
	scene->AddModel(&bodies[bVenus]);
	scene->AddModel(&bodies[bEarth]);
	scene->AddModel(&bodies[bMars]);
	scene->AddModel(&bodies[bJupiter]);
	scene->AddModel(&bodies[bSaturn]);
	scene->AddModel(&bodies[bUranus]);
	scene->AddModel(&bodies[bNeptune]);
	scene->AddModel(&saturnrings);
	scene->AddModel(&bodies[bSun]);
	scene->AddModel(&bodies[bMoon]);
	scene->AddModel(&bodies[bIo]);
	scene->AddModel(&bodies[bEuropa]);
	scene->AddModel(&stars);

	// Add the visual orbits to the renderer
	for (int i = 0; i < 8; ++i)
	{
		scene->AddModel(&orbits[i]);
	}
}


/**
 * Routines responsible for rendering. Called once per frame.
 */
void Display()
{
	scene->Optimize();
	pipeline->Render();
}

int selectedBodyIndex = 3;
int callCnt = 0;
int sp = 0, np = 0, aa = 0, pp = 0;
int brightness = 50;
int shadowMapQuality = 1;
bool normalMappingEnabled = true;
bool orbitsEnabled = true;
const char* itemList = "Low (512px)\0Medium (768px)\0High (1024px)\0Very High (2048px)\0";

bool showSimInfo = true;
bool showGraphicOptions = true;
bool showPlanetInfo = true;
void DisplayUi()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 4.0f;
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.1f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.2f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.4f);

	if (callCnt % 15 == 0)
	{
		sp = (sp + pipeline->GetTimingShadowPass()) / 2000;
		np = (np + pipeline->GetTimingNormalPass()) / 2000;
		aa = (aa + pipeline->GetTimingAntialiasing()) / 2000;
		pp = (pp + pipeline->GetTimingPostProcessing()) / 2000;
	}
	++callCnt;

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("View", true))
	{
		ImGui::MenuItem("Sim. Info", NULL, &showSimInfo);
		ImGui::MenuItem("Options", NULL, &showGraphicOptions);
		ImGui::MenuItem("Selection", NULL, &showPlanetInfo);
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();

	if (showSimInfo)
	{
		if (ImGui::Begin("Info", &showSimInfo))
		{
			ImGui::Text("%-12s %d (%.1f ms)", "fps:", fpsCounter.GetFPS(), fpsCounter.GetTimeForFrame());
			ImGui::Text("%-12s %.1f days", "time:", (float)simulationTime);
			ImGui::Text("%-12s %.1f days/s", "sim speed:", simulationTick * SIMULATION_FREQ);

			ImGui::Text("\r\n%-12s %d us", "shadow:", sp);
			ImGui::Text("%-12s %d us", "render:", np);
			ImGui::Text("%-12s %d us", "msaa:", aa);
			ImGui::Text("%-12s %d us", "post:", pp);
		}
		ImGui::End();
	}

	if (showGraphicOptions)
	{
		if (ImGui::Begin("Options", &showGraphicOptions))
		{
			if (ImGui::Combo("Shadow", &shadowMapQuality, itemList))
			{
				switch (shadowMapQuality)
				{
				case 0: pipeline->ChangeShadowmapSize(512); break;
				case 1: pipeline->ChangeShadowmapSize(768); break;
				case 2: pipeline->ChangeShadowmapSize(1024); break;
				case 3: pipeline->ChangeShadowmapSize(2048); break;
				default: break;
				}
			}
			if (ImGui::SliderInt("Brightness", &brightness, 0, 100, "%.0f%%"))
			{
				pipeline->SetBrightness(brightness);

				float ambC = 0.001f + brightness * 0.005f / 100.0f;
				sunLight->SetColor(LightComponent::AMBIENT, ambC, ambC, ambC);
			}
			if (ImGui::Checkbox("Normal Mapping", &normalMappingEnabled))
			{
				bodies[bMercury].SetNormalMapUsage(0, normalMappingEnabled);
				bodies[bEarth].SetNormalMapUsage(0, normalMappingEnabled);
				bodies[bMars].SetNormalMapUsage(0, normalMappingEnabled);
				bodies[bMoon].SetNormalMapUsage(0, normalMappingEnabled);
			}
            if (ImGui::Checkbox("Show Orbits", &orbitsEnabled))
            {
                for (int i = 0; i < 8; ++i)
                {
                    orbitsEnabled 
                        ? scene->AddModel(&orbits[i]) 
                        : scene->RemoveModel(&orbits[i]);
                }
            }
		}
		ImGui::End();
	}

	if (showPlanetInfo)
	{
		if (ImGui::Begin("Selection", &showPlanetInfo))
		{
			int si = selectedBodyIndex;
			ImGui::Text("%-12s %s", "Name:", bodyNames[si]);
			ImGui::Text("%-12s %.1f", "Size:", planetInfo[si].planetSize);
			ImGui::Text("%-12s %.1f days around parent", "RTT:", planetInfo[si].roundTripTime);
			ImGui::Text("%-12s %.1f earth days/rotation", "Revs:", planetInfo[si].selfRotationTime);
			ImGui::Text("%-12s %.1f deg", "Equator inclination:", glm::degrees(planetInfo[si].equatorInclination));
			ImGui::Text("%-12s %.1f deg", "Orbit inclination:", glm::degrees(planetInfo[si].orbitInclination));
		}
		ImGui::End();
	}

    ImGui::Render();
}


/**
 * Routine responsible to calculate & update all objects state. => MVP matrix and other properties.
 * Called before rendering, once per frame, but with a time reference as parameter to be CPU speed independent.
 * @param time Number of elapsed simulation ticks. <SIMULATION_FREQ> ticks / s.
 */
void Update(double time)
{
	// Animate sun
	float move = (float)(fmod(time, (double)sunInfo.selfRotationTime));
    bodies[bSun].textureTransforms[1] = glm::translate(mat3(), vec2(move*1.5, 0.0f));
    bodies[bSun].modelMatrix = PlanetMovementSystem::OrbitAroundSun(time, sunInfo);
	

	// Update the planets position
    bodies[bMercury].modelMatrix =	PlanetMovementSystem::OrbitAroundSun(time, mercuryInfo);
    bodies[bVenus].modelMatrix =	PlanetMovementSystem::OrbitAroundSun(time, venusInfo);
    bodies[bEarth].modelMatrix =	PlanetMovementSystem::OrbitAroundSun(time, earthInfo);
    bodies[bMars].modelMatrix =		PlanetMovementSystem::OrbitAroundSun(time, marsInfo);
    bodies[bJupiter].modelMatrix =	PlanetMovementSystem::OrbitAroundSun(time, jupiterInfo);
    bodies[bSaturn].modelMatrix =	PlanetMovementSystem::OrbitAroundSun(time, saturnInfo);
    bodies[bUranus].modelMatrix =	PlanetMovementSystem::OrbitAroundSun(time, uranusInfo);
    bodies[bNeptune].modelMatrix =	PlanetMovementSystem::OrbitAroundSun(time, neptuneInfo);

	// Update the moons positions...
    bodies[bIo].modelMatrix =		PlanetMovementSystem::OrbitAroundParent(time, ioInfo, vec3(bodies[bJupiter].modelMatrix[3]));
    bodies[bEuropa].modelMatrix =	PlanetMovementSystem::OrbitAroundParent(time, europaInfo, vec3(bodies[bJupiter].modelMatrix[3]));
    bodies[bMoon].modelMatrix =		PlanetMovementSystem::OrbitAroundParent(time, moonInfo, vec3(bodies[bEarth].modelMatrix[3]));

	// ... and the saturn ring
	saturnrings.modelMatrix = PlanetMovementSystem::SimulateRing(time, saturnRing, vec3(bodies[bSaturn].modelMatrix[3]));
}


void DoPlanetSelection(glm::vec3 rayOrigin, glm::vec3 rayDirection)
{
    float smallestDistance = 999999.9f;
    for (int i = bMercury; i < bEnd; ++i)
    {
		float radius = planetInfo[i].planetSize;
        float d1, d2;
        if(jge::Util::RaySphereIntersection(rayOrigin, rayDirection, bodies[i].GetPosition(), radius, d1, d2) > 0)
        {
            if (d1 > 0.0f && d1 < smallestDistance)
            {
                smallestDistance = d1;
                selectedBodyIndex = i;
            }
        }
    }
}


//////////////////////////////////////////// UTILITY FUNCTIONS //////////////////////////////////////////////

void* ReadEntireFile(const char* filename, int* length)
{
	void* buffer;
	FILE* f = fopen(filename, "r");

	if (!f)
	{
		char buffer[256];
		sprintf(buffer,"Cannot open or find %s", filename);
		throw std::runtime_error(buffer);
	}

	
	fseek(f, 0, SEEK_END);
	*length = ftell(f);
	fseek(f, 0, SEEK_SET);

	buffer = malloc(*length + 1);
	*length = fread(buffer, 1, *length, f);
	fclose(f);
	((char*)buffer)[*length] = '\0';

	return buffer;
}

GLuint LoadShader(GLenum type, const char* path)
{
	int length;
	char* data = (char*)ReadEntireFile(path, &length);
	try
	{
		return ShaderProgram::CreateShader(type, data, length);
	}
	catch (const std::runtime_error& ex)
	{
		throw std::runtime_error(std::string(path) + std::string("\r\n") + std::string(ex.what()));
	}
}

GLuint LoadTexture(const char* file)
{
	GLuint tex = Texture::LoadTexture(file);
	if (tex <= 0)
	{
		char buffer[256];
		sprintf(buffer,"Cannot load texture %s.", file);
		throw std::runtime_error(buffer);
	}
	return tex;
}

GLuint LoadTexture(const char* file, GLuint wrapMode, bool srgbInternal)
{
	GLuint tex = Texture::LoadTexture(file, wrapMode, srgbInternal);
	if (tex <= 0)
	{
		char buffer[256];
		sprintf(buffer, "Cannot load texture %s.", file);
		throw std::runtime_error(buffer);
	}
	return tex;
}

void LoadStarCatalog(Mesh& m)
{
    int numStars = 9110;
    int numElements = numStars * 3;

    float* vertArray = new float[numElements];
    float* colorArray = new float[numElements];

    FILE* fp = fopen("models\\starcatalog_float.bin", "rb");
    if (fp == NULL)
    {
        throw std::runtime_error("Cannot open starcatalog!");
    }

    fread(vertArray, numElements * sizeof(float), 1, fp);
    fread(colorArray, numElements * sizeof(float), 1, fp);
    fclose(fp);


    m.Create(GL_POINTS, GL_STATIC_DRAW);
    std::vector<vec3>* verts = m.GetVertices();
    std::vector<vec3>* colors = m.GetNormals();

    verts->insert(verts->begin(), (vec3*)vertArray, ((vec3*)vertArray) + numStars);
    colors->insert(colors->begin(), (vec3*)colorArray, ((vec3*)colorArray) + numStars);
    m.Update();

    delete[] vertArray;
    delete[] colorArray;
}