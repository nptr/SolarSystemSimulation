#pragma once

#include "../gl_core_3_3.h"
#include "Framebuffer.h"
#include "Scene.h"
#include "Measurement.h"

#include <glm/glm.hpp>

namespace jge
{
	class ShaderProgram;
	class Model;
	class Mesh;

	class RenderPipeline
	{
	public:
		RenderPipeline(jge::Scene* _scene);
		~RenderPipeline();

		// Sets the size of the resulting image
		void SetSize(int w, int h);

		// Supplies all shaders to the pipeline
		void SupplyShaders(
			jge::ShaderProgram* shadow,
			jge::ShaderProgram* lighting,
			jge::ShaderProgram* blur,
			jge::ShaderProgram* blend,
			jge::ShaderProgram* skybox,
			jge::ShaderProgram* glow,
			jge::ShaderProgram* star
			);

		// Create all buffers used to render the scene. call last
		void Build();
		void ChangeShadowmapSize(int shadowMapSz);
		void SetBrightness(int brightness);

		// Renders the scene
		void Render();

		unsigned int GetTimingShadowPass() const;
		unsigned int GetTimingNormalPass() const;
		unsigned int GetTimingAntialiasing() const;
		unsigned int GetTimingPostProcessing() const;

	private:
		// Positions the lights in the world
		void UpdateLights();
		void DrawModelMinimal(ShaderProgram& shader, Model& m, int lvl);
		void DrawModel(ShaderProgram& shader, Model& m, int lvl);

		// Returns the matrices for a specific direction (cube map face)
		void GetLightMatrices(int dir, glm::vec3 lightPos, glm::mat4& projectionMatrix, glm::mat4& viewMatrix);

		void DrawShadowCastingModels(jge::ShaderProgram* shader);
		void DrawNonShadowCastingModels(jge::ShaderProgram* shader);

		// Lighting with Shadow
		void ShadowPass();
		void NormalPass(GLuint renderTarget);

		// Glow Effect
		void RenderGlowMap();
		void Blur(const Framebuffer& target, int sizeW, int sizeH);
		void ApplyGlow();

		jge::ShaderProgram* shadowShader;
		jge::ShaderProgram* lightingShader;
		jge::ShaderProgram* blurShader;
		jge::ShaderProgram* blendShader;

		jge::ShaderProgram* skyboxShader;
		jge::ShaderProgram* glowmapShader;
		jge::ShaderProgram* starShader;
		jge::Scene* scene;
		jge::Mesh fullscreenQuad;			// used in blur ping-pong and skybox rendering

		int sceneW, sceneH;					// Scene size aka. viewport size
		int shadowmapSize;
		int glowW, glowH;
		int brightness;

		// Glow
		Framebuffer sceneFramebuffer;		// 1: rendertarget for the scene	2: render together with blurred glowbuffer to screen
		Framebuffer glowFramebuffer;		// render the scene to glowbuffer with low intensity clamp
		Framebuffer blurFramebuffer;		// render to glowbuffer to blurbuffer and blur horizontal
											// render the blurbuffer back to the glowbuffer and blur vertical
		Framebuffer msaaResultFramebuffer;

		// Framebuffer Object (FBO) for the Shadows - One FBO per lightsource!
		CubeFramebuffer shadowCubes[MAX_LIGHTS];

		OpenGlTimer timerShadowPass;
		OpenGlTimer timerNormalPass;
		OpenGlTimer timerMultisampling;
		OpenGlTimer timerPostprocessing;

		unsigned int timeElapsedShadowPass;
		unsigned int timeElapsedNormalPass;
		unsigned int timeElapsedMSAA;
		unsigned int timeElapsedPost;
	};
}