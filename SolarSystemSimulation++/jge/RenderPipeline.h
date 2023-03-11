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

        void DrawShadowCastingModels(jge::ShaderProgram& shader);
        void DrawNonShadowCastingModels(jge::ShaderProgram& shader);

        // Lighting with Shadow
        void ShadowPass();
        void NormalPass(GLuint renderTarget);

        // Glow Effect
        void RenderGlowMap();
        void Blur(const Framebuffer& target, int sizeW, int sizeH);
        void ApplyGlow();

        jge::ShaderProgram* m_shadowShader;
        jge::ShaderProgram* m_lightingShader;
        jge::ShaderProgram* m_blurShader;
        jge::ShaderProgram* m_blendShader;

        jge::ShaderProgram* m_skyboxShader;
        jge::ShaderProgram* m_glowmapShader;
        jge::ShaderProgram* m_starShader;
        jge::Scene* m_scene;
        jge::Mesh m_fullscreenQuad;			// Used in blur ping-pong and skybox rendering

        int m_sceneW, m_sceneH;				// Scene size aka. viewport size
        int m_shadowmapSize;
        int m_glowW, m_glowH;
        int m_brightness;

        // Glow
        Framebuffer m_sceneFramebuffer;		// 1: Rendertarget for the scene	2: render together with blurred glowbuffer to screen
        Framebuffer m_glowFramebuffer;		// Render the scene to glowbuffer with low intensity clamp
        Framebuffer m_blurFramebuffer;		// Render the glowbuffer to blurbuffer and blur horizontal
                                            // Render the blurbuffer back to the glowbuffer and blur vertical
        Framebuffer m_msaaResultFramebuffer;

        // Framebuffer Object (FBO) for the Shadows - One FBO per lightsource!
        CubeFramebuffer m_shadowCubes[MAX_LIGHTS];

        OpenGlTimer m_timerShadowPass;
        OpenGlTimer m_timerNormalPass;
        OpenGlTimer m_timerMultisampling;
        OpenGlTimer m_timerPostprocessing;

        unsigned int m_timeElapsedShadowPass;
        unsigned int m_timeElapsedNormalPass;
        unsigned int m_timeElapsedMSAA;
        unsigned int m_timeElapsedPost;
    };
}