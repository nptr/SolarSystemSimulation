#pragma once

#include "Mesh.h"
#include "Texture.h"
#include <glm\glm.hpp>

#define MAX_LOD_LEVELS 3
#define MAX_TEXTURES 2

namespace jge
{
	class ShaderProgram;

    /**
    * Naive approach to representing renderable objects. It probably
    * doesn't scale well and will evolve into a god object..
    */
	class Model
	{
	public:
		Model();
		Model(Mesh* defaultMesh, GLuint textureId);
		~Model();

		void SetMeshes(Mesh* lod1);
		void SetMeshes(Mesh* lod1, Mesh* lod2);
		void SetMeshes(Mesh* lod1, Mesh* lod2, Mesh* lod3);

		void Draw(int lodLevel);
		void Draw();

		glm::vec3 GetPosition() const;

		void SetLODMesh(Mesh* m, int level);
		Mesh* GetLODMesh(int level) const;

		void SetTexture(GLuint texID);
		void SetTexture(GLuint texID, int num);
		GLuint GetTexture(int num) const;

        void SetTextureUsage(bool);
        bool IsUsingTexture() const;

		void SetNormalMap(GLuint texID);
		GLuint GetNormalMap() const;

		void SetSpecularMap(GLuint texID);
		GLuint GetSpecularMap() const;

		void SetColor(glm::vec4 color);
		glm::vec4 GetColor() const;

		void SetBlendMode(BlendMode mode);
		BlendMode GetBlendMode() const;


		bool IsTransparent() const;
		void SetTransparent(bool flag);

        bool IsSortingTrianglesRequired() const;
        void SetSortTriangles(bool flag);

		bool IsCastingShadow() const;
		void SetShadowCasting(bool);

        bool IsAffectedByLighting() const;
        void SetAffectedByLighting(bool);

		bool IsGlowing() const;
		void SetGlowEffect(bool);

		bool IsUsingNormalMap(int lodlvl) const;
		void SetNormalMapUsage(int lodlvl, bool use);

		bool IsUsingSpecularMap(int lodlvl) const;
		void SetSpecularMapUsage(int lodlvl, bool use);


		void SetShader(ShaderProgram* shdr);
		ShaderProgram* GetShader();


        inline float& DistanceToCamera() { return m_distToCam; }


		glm::mat4 modelMatrix;
		glm::mat3 textureTransforms[MAX_TEXTURES];
		GLuint textures[MAX_TEXTURES];
		

	private:
        Mesh * m_meshes[MAX_LOD_LEVELS];
        GLuint m_normalMap;
        GLuint m_specularMap;
        glm::vec4 m_modelColor;
        BlendMode m_blendMode;

        bool m_useTexture;
        bool m_renderAsTransparent;
        bool m_sortTriangles;
		bool m_isShadowCaster;
		bool m_isAffectedByLighting;
        bool m_isGlowing;
        bool m_useNormalMap[MAX_LOD_LEVELS];
        bool m_useSpecularMap[MAX_LOD_LEVELS];

        ShaderProgram* m_shader;

        float m_distToCam;
	};
}
