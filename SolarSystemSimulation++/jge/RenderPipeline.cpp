#include "RenderPipeline.h"
#include "ShaderProgram.h"
#include "Model.h"
#include "Mesh.h"

#include <glm/gtx/transform.hpp>

using namespace jge;
using namespace glm;

// World-View-Projection Matrix uniforms
const std::string UF_VIEW_MATRIX = "view";
const std::string UF_PROJECTION_MATRIX = "proj";
const std::string UF_MODEL_MATRIX = "model";

const std::string UF_BLENDMODE = "blendMode";
const std::string UF_TEXSAMPLER[] = { "textureSampler0", "textureSampler1" };
const std::string UF_TEX_TRANSFORM[] = { "texTransform[0]", "texTransform[1]" };
const std::string UF_USETEX[] = { "useTexture[0]", "useTexture[1]" };

// Mastershader options
const std::string UF_LIGHT_NUM = "numberOfLights";
const std::string UF_LIGHT_DISABLE = "excludeFromLighting";
const std::string UF_SOLIDCOLOR = "modelColor";
const std::string UF_SOLIDCOLOR_ENABLE = "useColor";
const std::string UF_NORMALMAPPING_ENABLE = "useNormalMapping";
const std::string UF_NORMALSAMPLER = "normalTexSampler";
const std::string UF_SPECULARMAPPING_ENABLE = "useSpecularMapping";
const std::string UF_SPECULARSAMPLER = "specularTexSampler";
const std::string UF_SHADOWSAMPLER[] = { "shadowCubes[0]", "shadowCubes[1]", "shadowCubes[2]", "shadowCubes[3]" };
const std::string UF_VIEW_POS = "viewPosition";

// Shadowshader options
const std::string UF_LIGHT_VIEW = "cameraToShadowView";
const std::string UF_LIGHT_PROJECTION = "cameraToShadowProjector";

// Skybox options
const std::string UF_BRIGHTNESS = "brightness";

// Blurshader options
const std::string UF_BLURSCALE = "ScaleU";
const std::string UF_SKYBOX_SAMPLER = "skyboxTex";

// Uniform Map for the lights. Memory vs. Performance - lookup is faster than string concat.
const std::string umapLightPosition[] = { "lights[0].position", "lights[1].position", "lights[2].position", "lights[3].position" };
const std::string umapAmbientColor[] = { "lights[0].ambient", "lights[1].ambient", "lights[2].ambient", "lights[3].ambient" };
const std::string umapDiffuseColor[] = { "lights[0].diffuse", "lights[1].diffuse", "lights[2].diffuse", "lights[3].diffuse" };
const std::string umapSpecularColor[] = { "lights[0].specular", "lights[1].specular", "lights[2].specular", "lights[3].specular" };

const std::string umapConstAtt[] = { "lights[0].constantAttenuation", "lights[1].constantAttenuation", "lights[2].constantAttenuation", "lights[3].constantAttenuation" };
const std::string umapLinAtt[] = { "lights[0].linearAttenuation", "lights[1].linearAttenuation", "lights[2].linearAttenuation", "lights[3].linearAttenuation" };
const std::string umapQuadAtt[] = { "lights[0].quadraticAttenuation", "lights[1].quadraticAttenuation", "lights[2].quadraticAttenuation", "lights[3].quadraticAttenuation" };

const std::string umapSpotCosCutoff[] = { "lights[0].spotCosCutoff", "lights[1].spotCosCutoff", "lights[2].spotCosCutoff", "lights[3].spotCosCutoff" };
const std::string umapSpotExponent[] = { "lights[0].spotExponent", "lights[1].spotExponent", "lights[2].spotExponent", "lights[3].spotExponent" };
const std::string umapSpotDirection[] = { "lights[0].spotDirection", "lights[1].spotDirection", "lights[2].spotDirection", "lights[3].spotDirection" };

#define MSAA_SAMPLES 4

RenderPipeline::RenderPipeline(Scene* _scene)
    : m_shadowShader(nullptr)
    , m_lightingShader(nullptr)
    , m_blurShader(nullptr)
    , m_blendShader(nullptr)
    , m_skyboxShader(nullptr)
    , m_glowmapShader(nullptr)
    , m_starShader(nullptr)
    , m_sceneW(0), m_sceneH(0)
    , m_shadowmapSize(768)
    , m_glowW(0), m_glowH(0)
    , m_scene(_scene)
    , m_brightness(60)
    , m_sceneFramebuffer()
    , m_glowFramebuffer()
    , m_blurFramebuffer()
    , m_msaaResultFramebuffer()
    , m_timeElapsedShadowPass(0)
    , m_timeElapsedNormalPass(0)
    , m_timeElapsedMSAA(0)
    , m_timeElapsedPost(0)
{
    // for the multisampled scene framebuffer
    glEnable(GL_MULTISAMPLE);

    // Enable depthtest
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable seamless cube map filtering
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_fullscreenQuad.CreateQuad();
}

RenderPipeline::~RenderPipeline()
{
    FramebufferTools::DeleteFramebuffer(m_sceneFramebuffer);
    FramebufferTools::DeleteFramebuffer(m_glowFramebuffer);
    FramebufferTools::DeleteFramebuffer(m_blurFramebuffer);
    FramebufferTools::DeleteFramebuffer(m_msaaResultFramebuffer);

    for (unsigned int i = 0; i < m_scene->lights.size(); ++i)
    {
        FramebufferTools::DeleteFramebufferCube(m_shadowCubes[i]);
    }
}

void RenderPipeline::SetBrightness(int brightness)
{
    m_brightness = brightness;
}

void RenderPipeline::SetSize(int w, int h)
{
    if (w == m_sceneW && h == m_sceneH)
        return;

    m_sceneW = w;
    m_sceneH = h;
    m_glowW = m_sceneW / 4;
    m_glowH = m_sceneH / 4;

    FramebufferTools::DeleteFramebuffer(m_sceneFramebuffer);
    m_sceneFramebuffer = FramebufferTools::CreateFramebuffer(m_sceneW, m_sceneH, true, MSAA_SAMPLES);

    FramebufferTools::DeleteFramebuffer(m_msaaResultFramebuffer);
    m_msaaResultFramebuffer = FramebufferTools::CreateFramebuffer(m_sceneW, m_sceneH, true);

    FramebufferTools::DeleteFramebuffer(m_glowFramebuffer);
    FramebufferTools::DeleteFramebuffer(m_blurFramebuffer);
    m_glowFramebuffer = FramebufferTools::CreateFramebuffer(m_glowW, m_glowH, true);
    m_blurFramebuffer = FramebufferTools::CreateFramebuffer(m_glowW, m_glowH, false);
}

void RenderPipeline::Build()
{
    // Shadow Map
    m_lightingShader->UseProgram();
    m_lightingShader->UpdateUniform(UF_LIGHT_NUM, (int)m_scene->lights.size());

    for (unsigned int i = 0; i < m_scene->lights.size(); ++i)
    {
        // Create a shadow cube for the light. TODO: Create simple texures for spotlights...
        GLuint tex = FramebufferTools::MakeCubeTexture_Color(m_shadowmapSize);
        GLuint depth = FramebufferTools::MakeCubeTexture_Depth(m_shadowmapSize);
        m_shadowCubes[i] = FramebufferTools::MakeFramebufferCube(tex, depth);
    }

    m_sceneFramebuffer = FramebufferTools::CreateFramebuffer(m_sceneW, m_sceneH, true, MSAA_SAMPLES);
    m_msaaResultFramebuffer = FramebufferTools::CreateFramebuffer(m_sceneW, m_sceneH, true);	// has to have a depth buffer too!
    m_glowFramebuffer = FramebufferTools::CreateFramebuffer(m_glowW, m_glowH, true);
    m_blurFramebuffer = FramebufferTools::CreateFramebuffer(m_glowW, m_glowH, false);
}

void RenderPipeline::SupplyShaders(
    jge::ShaderProgram* shadow,
    jge::ShaderProgram* lighting,
    jge::ShaderProgram* blur,
    jge::ShaderProgram* blend,
    jge::ShaderProgram* skybox,
    jge::ShaderProgram* glow,
    jge::ShaderProgram* star
)
{
    m_shadowShader = shadow;
    m_lightingShader = lighting;
    m_lightingShader->UseProgram();
    m_lightingShader->UpdateUniform(UF_SHADOWSAMPLER[0], 0);
    m_lightingShader->UpdateUniform(UF_SHADOWSAMPLER[1], 1);
    m_lightingShader->UpdateUniform(UF_SHADOWSAMPLER[2], 2);
    m_lightingShader->UpdateUniform(UF_SHADOWSAMPLER[3], 3);
    m_lightingShader->UpdateUniform(UF_TEXSAMPLER[0], 4);
    m_lightingShader->UpdateUniform(UF_TEXSAMPLER[1], 5);
    m_lightingShader->UpdateUniform(UF_NORMALSAMPLER, 6);
    m_lightingShader->UpdateUniform(UF_SPECULARSAMPLER, 7);

    m_blurShader = blur;
    m_blurShader->UseProgram();
    m_blurShader->UpdateUniform(UF_TEXSAMPLER[0], 0);

    m_blendShader = blend;
    m_blendShader->UseProgram();
    m_blendShader->UpdateUniform(UF_TEXSAMPLER[0], 0);
    m_blendShader->UpdateUniform(UF_TEXSAMPLER[1], 1);

    m_skyboxShader = skybox;
    m_skyboxShader->UseProgram();
    m_skyboxShader->UpdateUniform(UF_SKYBOX_SAMPLER, 0);

    m_glowmapShader = glow;
    m_starShader = star;
}

void RenderPipeline::ChangeShadowmapSize(int newSize)
{
    m_shadowmapSize = newSize;
    for (unsigned int i = 0; i < m_scene->lights.size(); ++i)
    {
        FramebufferTools::DeleteFramebufferCube(m_shadowCubes[i]);

        GLuint tex = FramebufferTools::MakeCubeTexture_Color(m_shadowmapSize);
        GLuint depth = FramebufferTools::MakeCubeTexture_Depth(m_shadowmapSize);
        m_shadowCubes[i] = FramebufferTools::MakeFramebufferCube(tex, depth);
    }
}

void RenderPipeline::UpdateLights()
{
    m_lightingShader->UseProgram();
    for (unsigned int i = 0; i < m_scene->lights.size(); ++i)
    {
        float constAtt;
        float linearAtt;
        float quadricAtt;
        m_scene->lights[i]->GetAttenuation(constAtt, linearAtt, quadricAtt);

        float cutoff;
        float exponent;
        vec3 direction;
        m_scene->lights[i]->GetSpotProperties(cutoff, exponent, direction);

        m_lightingShader->UpdateUniform(umapLightPosition[i], m_scene->lights[i]->GetPosition());
        m_lightingShader->UpdateUniform(umapAmbientColor[i], m_scene->lights[i]->GetColor(LightComponent::AMBIENT));
        m_lightingShader->UpdateUniform(umapDiffuseColor[i], m_scene->lights[i]->GetColor(LightComponent::DIFFUSE));
        m_lightingShader->UpdateUniform(umapSpecularColor[i], m_scene->lights[i]->GetColor(LightComponent::SPECULAR));
        m_lightingShader->UpdateUniform(umapConstAtt[i], constAtt);
        m_lightingShader->UpdateUniform(umapLinAtt[i], linearAtt);
        m_lightingShader->UpdateUniform(umapQuadAtt[i], quadricAtt);
        m_lightingShader->UpdateUniform(umapSpotCosCutoff[i], cutoff);	// shader checks for cos(180°) = -1;
        m_lightingShader->UpdateUniform(umapSpotExponent[i], exponent);
        m_lightingShader->UpdateUniform(umapSpotDirection[i], direction);
    }
}


void RenderPipeline::GetLightMatrices(int dir, glm::vec3 lightPos, glm::mat4& projectionMatrix, glm::mat4& viewMatrix)
{
    glm::mat4 mat, view;

    mat *= glm::perspective(glm::radians(90.0f), 1.0f, 0.2f, 300.0f);
    switch (dir) {
    case 0:
        // +X
        view = glm::lookAt(lightPos, lightPos + glm::vec3(+1, 0, 0), glm::vec3(0, -1, 0));
        mat *= view;
        break;
    case 1:
        // -X
        view = glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
        mat *= view;
        break;
    case 2:
        // +Y
        view = glm::lookAt(lightPos, lightPos + glm::vec3(0, +1, 0), glm::vec3(0, 0, -1));
        mat *= view;
        break;
    case 3:
        // -Y
        view = glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
        mat *= view;
        break;
    case 4:
        // +Z
        view = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, +1), glm::vec3(0, -1, 0));
        mat *= view;
        break;
    case 5:
        // -Z
        view = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
        mat *= view;
        break;
    default:	// Do nothing
        break;
    }

    projectionMatrix = mat;
    viewMatrix = view;
}


void RenderPipeline::DrawShadowCastingModels(ShaderProgram& shader)
{
    Model* m;
    glEnable(GL_CULL_FACE);
    for (unsigned int i = 0; i < m_scene->shadowCastingModels.size(); ++i)
    {
        m = m_scene->shadowCastingModels[i];
        shader.UpdateUniform(UF_MODEL_MATRIX, m->modelMatrix);
        int lvl = m_scene->DetermineLODLevel(*m);
        m->Draw(lvl);
    }
}

void RenderPipeline::DrawModel(ShaderProgram& shader, Model& m, int lvl)
{
    // bind texture 1 (multitexturing)
    bool useMultitexturing = m.textures[1] > 0;
    shader.UpdateUniform(UF_USETEX[1], useMultitexturing);
    if (useMultitexturing)
    {
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, m.textures[1]);
        shader.UpdateUniform(UF_TEX_TRANSFORM[1], m.textureTransforms[1]);
        shader.UpdateUniform(UF_BLENDMODE, (int)m.GetBlendMode());
    }

    // use texture unit 6 for the normal map
    bool useNormalMap = m.IsUsingNormalMap(lvl);
    shader.UpdateUniform(UF_NORMALMAPPING_ENABLE, useNormalMap);
    if (useNormalMap)
    {
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, m.GetNormalMap());
    }

    // use texture unit 7 for the specular map
    bool useSpecularMap = m.IsUsingSpecularMap(lvl);
    shader.UpdateUniform(UF_SPECULARMAPPING_ENABLE, useSpecularMap);
    if (useSpecularMap)
    {
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, m.GetSpecularMap());
    }

    // extended featureset
    shader.UpdateUniform(UF_SOLIDCOLOR_ENABLE, !m.IsUsingTexture());
    shader.UpdateUniform(UF_SOLIDCOLOR, m.GetColor());
    shader.UpdateUniform(UF_LIGHT_DISABLE, !m.IsAffectedByLighting());

    DrawModelMinimal(shader, m, lvl);
}

void RenderPipeline::DrawModelMinimal(ShaderProgram& shader, Model& m, int lvl)
{
    // bind default texture 0
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m.textures[0]);

    // set tex transform
    shader.UpdateUniform(UF_TEX_TRANSFORM[0], m.textureTransforms[0]);
    shader.UpdateUniform(UF_USETEX[0], 0);

    // upload model position
    shader.UpdateUniform(UF_MODEL_MATRIX, m.modelMatrix);
    m.Draw(lvl);
}

void RenderPipeline::DrawNonShadowCastingModels(ShaderProgram& defaultShader)
{
    Model* m;
    ShaderProgram* currentShader = nullptr;
    ShaderProgram* lastShader = nullptr;


    // Render front to back to allow early Z test
    // unsigned iterator, beware wrap around
    for (unsigned int i = m_scene->opaqueModels.size(); i--;)
    {
        m = m_scene->opaqueModels[i];

        currentShader = m->GetShader();
        if (currentShader == nullptr)
            currentShader = &defaultShader;

        if (currentShader != lastShader)
        {
            currentShader->UseProgram();
            currentShader->UpdateUniform(UF_VIEW_MATRIX, m_scene->camera->GetViewMatrix());
            currentShader->UpdateUniform(UF_PROJECTION_MATRIX, m_scene->camera->GetProjectionMatrix());
        }

        int lvl = m_scene->DetermineLODLevel(*m);
        DrawModel(*currentShader, *m, lvl);

        lastShader = currentShader;
    }

    glDisable(GL_CULL_FACE);

    // Render transparent models back to front for correct visibility
    for (unsigned int i = 0; i < m_scene->transparentModels.size(); ++i)
    {
        m = m_scene->transparentModels[i];

        currentShader = m->GetShader();
        if (currentShader == nullptr)
            currentShader = &defaultShader;

        if (currentShader != lastShader)
        {
            currentShader->UseProgram();
            // normal draw is too much, minimal not enough...
            currentShader->UpdateUniform(UF_LIGHT_DISABLE, !m->IsAffectedByLighting());
            currentShader->UpdateUniform(UF_VIEW_MATRIX, m_scene->camera->GetViewMatrix());
            currentShader->UpdateUniform(UF_PROJECTION_MATRIX, m_scene->camera->GetProjectionMatrix());
        }

        int lvl = m_scene->DetermineLODLevel(*m);
        DrawModel(*currentShader, *m, lvl);

        lastShader = currentShader;
    }
}

void RenderPipeline::ShadowPass()
{
    glDisable(GL_BLEND);
    glViewport(0, 0, m_shadowmapSize, m_shadowmapSize);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shadowShader->UseProgram();

    // For each light, we render from every 6 sides into the framebuffer
    // in order to determine the shadowed areas in the shader later.
    for (unsigned int i = 0; i < m_scene->lights.size() && i < MAX_LIGHTS; ++i)
    {
        // For each side of the cubemap (framebuffercube)
        for (int side = 0; side < 6; ++side)
        {
            // bind side
            glBindFramebuffer(GL_FRAMEBUFFER, m_shadowCubes[i].handle[side]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // position ourselfs on one side and render
            glm::mat4 lightViewMatrix, lightProjectionMatrix;
            GetLightMatrices(side, vec3(m_scene->lights[i]->GetPosition()), lightProjectionMatrix, lightViewMatrix);
            m_shadowShader->UpdateUniform(UF_LIGHT_VIEW, lightViewMatrix);
            m_shadowShader->UpdateUniform(UF_LIGHT_PROJECTION, lightProjectionMatrix);

            DrawShadowCastingModels(*m_shadowShader);
        }
    }
}

void RenderPipeline::NormalPass(GLuint renderTarget)
{
    glEnable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget);

    glViewport(0, 0, m_sceneW, m_sceneH);
    glCullFace(GL_BACK);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render Skybox
    glDepthMask(GL_FALSE);
    m_skyboxShader->UseProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_scene->skyBoxTexture);
    m_skyboxShader->UpdateUniform(UF_BRIGHTNESS, m_brightness * 0.01f / 100.0f);
    m_skyboxShader->UpdateUniform(UF_VIEW_MATRIX, m_scene->camera->GetViewMatrix());
    m_skyboxShader->UpdateUniform(UF_PROJECTION_MATRIX, m_scene->camera->GetProjectionMatrix());
    m_fullscreenQuad.Draw();
    glDepthMask(GL_TRUE);

    // Update Camera / World Position
    m_lightingShader->UseProgram();
    m_lightingShader->UpdateUniform(UF_VIEW_MATRIX, m_scene->camera->GetViewMatrix());
    m_lightingShader->UpdateUniform(UF_PROJECTION_MATRIX, m_scene->camera->GetProjectionMatrix());
    m_lightingShader->UpdateUniform(UF_VIEW_POS, glm::vec4(m_scene->camera->GetPosition(), 0.0f));
    // Update Light Position and Uniforms
    this->UpdateLights();

    // Bind ShadowCubes to the Texture Units
    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowCubes[i].colorTex);
    }

    DrawNonShadowCastingModels(*m_lightingShader);
}

void RenderPipeline::RenderGlowMap()
{
    m_glowmapShader->UseProgram();
    m_glowmapShader->UpdateUniform(UF_VIEW_MATRIX, m_scene->camera->GetViewMatrix());
    m_glowmapShader->UpdateUniform(UF_PROJECTION_MATRIX, m_scene->camera->GetProjectionMatrix());

    glBindFramebuffer(GL_FRAMEBUFFER, m_glowFramebuffer.handle);

    glViewport(0, 0, m_glowW, m_glowH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render occluding objects but disable color writes
    // Only depth buffer gets written
    glColorMask(false, false, false, false);
    Model* m;
    for (unsigned int i = 0; i < m_scene->nonglowingModels.size(); ++i)
    {
        m = m_scene->nonglowingModels[i];

        int lvl = m_scene->DetermineLODLevel(*m);
        DrawModelMinimal(*m_glowmapShader, *m, lvl);
    }
    glColorMask(true, true, true, true);

    // Render glowing objects, depth test will occlude glow
    for (unsigned int i = 0; i < m_scene->glowingModels.size(); ++i)
    {
        m = m_scene->glowingModels[i];
        m_glowmapShader->UpdateUniform(UF_SOLIDCOLOR, m->GetColor());

        int lvl = m_scene->DetermineLODLevel(*m);
        DrawModelMinimal(*m_glowmapShader, *m, lvl);
    }
}

void RenderPipeline::Blur(const Framebuffer& target, int targetSizeW, int targetSizeH)
{
    m_blurShader->UseProgram();
    glActiveTexture(GL_TEXTURE0);

    // Blur Pass 1 - X Axis
    m_blurShader->UpdateUniform(UF_BLURSCALE, vec2(1.0f / targetSizeW, 0.0f));

    glBindTexture(GL_TEXTURE_2D, target.colorTex);
    glBindFramebuffer(GL_FRAMEBUFFER, m_blurFramebuffer.handle);

    glViewport(0, 0, targetSizeW, targetSizeH);
    m_fullscreenQuad.Draw();

    // Blur Pass 2 - Y Axis
    m_blurShader->UpdateUniform(UF_BLURSCALE, vec2(0.0f, 1.0f / targetSizeH));

    glBindTexture(GL_TEXTURE_2D, m_blurFramebuffer.colorTex);
    glBindFramebuffer(GL_FRAMEBUFFER, target.handle);

    glViewport(0, 0, targetSizeW, targetSizeH);
    m_fullscreenQuad.Draw();
}

void RenderPipeline::ApplyGlow()
{
    m_blendShader->UseProgram();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_sceneW, m_sceneH);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_msaaResultFramebuffer.colorTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_glowFramebuffer.colorTex);

    m_fullscreenQuad.Draw();
}

void RenderPipeline::Render()
{
    m_timerShadowPass.Start();
    ShadowPass();
    m_timerShadowPass.Stop();

    m_timerNormalPass.Start();
    NormalPass(m_sceneFramebuffer.handle);
    m_timerNormalPass.Stop();

    m_timerMultisampling.Start();
    // Copy from multisampled FB to a normal one - used in ApplyGlow()
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_sceneFramebuffer.handle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_msaaResultFramebuffer.handle);
    glBlitFramebuffer(0, 0, m_sceneW, m_sceneH, 0, 0, m_sceneW, m_sceneH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    m_timerMultisampling.Stop();

    m_timerPostprocessing.Start();
    RenderGlowMap();
    Blur(m_glowFramebuffer, m_glowW, m_glowH);
    ApplyGlow();
    m_timerPostprocessing.Stop();

    m_timeElapsedShadowPass = m_timerShadowPass.GetElapsedTime();
    m_timeElapsedNormalPass = m_timerNormalPass.GetElapsedTime();
    m_timeElapsedMSAA = m_timerMultisampling.GetElapsedTime();
    m_timeElapsedPost = m_timerPostprocessing.GetElapsedTime();

    m_timerShadowPass.Swap();
    m_timerNormalPass.Swap();
    m_timerMultisampling.Swap();
    m_timerPostprocessing.Swap();
}

unsigned int RenderPipeline::GetTimingShadowPass() const { return m_timeElapsedShadowPass; }
unsigned int RenderPipeline::GetTimingNormalPass() const { return m_timeElapsedNormalPass; }
unsigned int RenderPipeline::GetTimingAntialiasing() const { return m_timeElapsedMSAA; }
unsigned int RenderPipeline::GetTimingPostProcessing() const { return m_timeElapsedPost; }
