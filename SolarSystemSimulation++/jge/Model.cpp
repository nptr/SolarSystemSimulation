#include "Model.h"
#include "ShaderProgram.h"
#include <vector>
#include "../gl_core_3_3.h"

using namespace glm;

namespace jge
{

    Model::Model() : Model(nullptr, 0)
    {}

    Model::Model(Mesh* defaultMesh, GLuint textureId)
        : m_isShadowCaster(true)
        , m_isAffectedByLighting(true)
        , m_sortTriangles(false)
        , m_renderAsTransparent(false)
        , modelMatrix(mat4())
        , m_useTexture(true)
        , m_isGlowing(false)
        , m_shader(nullptr)
        , m_blendMode(BlendMode::NORMAL)
    {
        m_meshes[0] = defaultMesh;
        m_meshes[1] = nullptr;
        m_meshes[2] = nullptr;
        textures[0] = textureId;
        textures[1] = 0;

        textureTransforms[0] = mat3();
        textureTransforms[1] = mat3();

        memset(&m_useNormalMap, 0, sizeof(bool) * MAX_LOD_LEVELS);
    }


    Model::~Model()
    {
    }

    void Model::SetMeshes(Mesh* lod1)
    {
        m_meshes[0] = lod1;
    }

    void Model::SetMeshes(Mesh* lod1, Mesh* lod2)
    {
        m_meshes[0] = lod1;
        m_meshes[1] = lod2;
    }

    void Model::SetMeshes(Mesh* lod1, Mesh* lod2, Mesh* lod3)
    {
        m_meshes[0] = lod1;
        m_meshes[1] = lod2;
        m_meshes[2] = lod3;
    }

    void Model::SetColor(vec4 color)
    {
        m_modelColor = color;
    }

    vec4 Model::GetColor() const
    {
        return m_modelColor;
    }

    void Model::SetTextureUsage(bool use)
    {
        m_useTexture = use;
    }

    bool Model::IsUsingTexture() const
    {
        return m_useTexture;
    }

    void Model::SetTexture(GLuint t)
    {
        textures[0] = t;
    }

    void Model::SetTexture(GLuint t, int num)
    {
        assert(num >= 0);
        assert(num < MAX_TEXTURES);
        textures[num] = t;
    }

    GLuint Model::GetTexture(int num) const
    {
        assert(num > 0);
        assert(num < MAX_TEXTURES);
        return textures[num];
    }

    void Model::SetBlendMode(BlendMode mode)
    {
        m_blendMode = mode;
    }

    BlendMode Model::GetBlendMode() const
    {
        return m_blendMode;
    }

    void Model::SetNormalMap(GLuint texID)
    {
        m_normalMap = texID;
    }

    GLuint Model::GetNormalMap() const
    {
        return m_normalMap;
    }

    void Model::SetSpecularMap(GLuint texID)
    {
        m_specularMap = texID;
    }

    GLuint Model::GetSpecularMap() const
    {
        return m_specularMap;
    }

    void Model::SetShadowCasting(bool va)
    {
        m_isShadowCaster = va;
    }

    bool Model::IsCastingShadow() const
    {
        return m_isShadowCaster;
    }

    void Model::SetAffectedByLighting(bool state)
    {
        m_isAffectedByLighting = state;
    }

    bool Model::IsAffectedByLighting() const
    {
        return m_isAffectedByLighting;
    }

    bool Model::IsTransparent() const
    {
        return m_renderAsTransparent;
    }

    void Model::SetTransparent(bool flag)
    {
        m_renderAsTransparent = flag;
    }

    bool Model::IsSortingTrianglesRequired() const
    {
        return m_sortTriangles;
    }

    void Model::SetSortTriangles(bool flag)
    {
        m_sortTriangles = flag;
    }

    bool Model::IsUsingNormalMap(int lodlvl) const
    {
        assert(lodlvl >= 0);
        assert(lodlvl < MAX_LOD_LEVELS);
        return m_useNormalMap[lodlvl];
    }

    void Model::SetNormalMapUsage(int lodlvl, bool use)
    {
        assert(lodlvl >= 0);
        assert(lodlvl < MAX_LOD_LEVELS);
        m_useNormalMap[lodlvl] = use;
    }

    bool Model::IsUsingSpecularMap(int lodlvl) const
    {
        assert(lodlvl >= 0);
        assert(lodlvl < MAX_LOD_LEVELS);
        return m_useSpecularMap[lodlvl];
    }

    void Model::SetSpecularMapUsage(int lodlvl, bool use)
    {
        assert(lodlvl >= 0);
        assert(lodlvl < MAX_LOD_LEVELS);
        m_useSpecularMap[lodlvl] = use;
    }

    bool Model::IsGlowing() const
    {
        return m_isGlowing;
    }

    void Model::SetGlowEffect(bool state)
    {
        m_isGlowing = state;
    }

    vec3 Model::GetPosition() const
    {
        vec3 modelPos(modelMatrix[3]);
        return modelPos;
    }

    void Model::SetShader(ShaderProgram* shdr)
    {
        m_shader = shdr;
    }

    ShaderProgram* Model::GetShader()
    {
        return m_shader;
    }

    Mesh* Model::GetLODMesh(int level) const
    {
        if (level == 2 && m_meshes[2] != NULL)
            return m_meshes[2];
        if (level >= 1 && m_meshes[1] != NULL)
            return m_meshes[1];
        return m_meshes[0];
    }

    void Model::SetLODMesh(Mesh* m, int level)
    {
        assert(level >= 0);
        assert(level < MAX_LOD_LEVELS);
        m_meshes[level] = m;
    }

    void Model::Draw(int lodLevel)
    {
        while (m_meshes[lodLevel] == nullptr)
            --lodLevel;

        m_meshes[lodLevel]->Draw();
    }

    void Model::Draw()
    {
        this->Draw(0);
    }
}