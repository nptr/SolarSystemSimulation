#pragma once

#include "../gl_core_3_3.h"
#include <glm/glm.hpp>
#include <vector>


namespace jge
{
    class Mesh
    {
    public:
        Mesh();
        ~Mesh();

        // Create the mesh manually or from a preset
        void Create(GLenum vertexType, GLenum usage, bool generateTangents = false);
        void CreateCircle();
        void CreateQuad();

        // Provides r/w access to the local mesh data
        inline std::vector<glm::vec3>& GetVertices() { return m_vertices; }
        inline std::vector<glm::vec2>& GetTexCoords2D() { return m_uvs; }
        inline std::vector<glm::vec3>& GetTexCoords3D() { return m_uvws; }
        inline std::vector<glm::vec3>& GetNormals() { return m_normals; }
        inline std::vector<glm::vec3>& GetTangents() { return m_tangents; }
        inline std::vector<glm::vec3>& GetBiTangents() { return m_bitangents; }

        GLuint GetVAO() const;

        // Uploads the mesh data to the GPU
        void Update();

        void Draw() const;
        void DrawNoBind() const;

        // Read a Wavefront OBJ file
        void FromObjectFile(const char* objFile, bool generateTangents = false);
        void SaveBufferToFile(const char* objFile);

    private:
        // Uploads the data to the GPU
        void SetBufferData(GLuint bufferID, const std::vector<glm::vec2>& data, GLenum usage);
        void SetBufferData(GLuint bufferID, const std::vector<glm::vec3>& data, GLenum usage);

        // Attached the buffers to the VAO
        void Attach(GLuint bufferID, int location, int size);

        void GenerateTangents();
        void Draw(GLenum mode) const;

        GLuint m_vertexArrayID;
        GLenum m_drawType;
        GLenum m_usageType;

        // The local data
        std::vector<glm::vec3> m_vertices;
        std::vector<glm::vec2> m_uvs;
        std::vector<glm::vec3> m_uvws;
        std::vector<glm::vec3> m_normals;
        std::vector<glm::vec3> m_tangents;
        std::vector<glm::vec3> m_bitangents;

        // The handle to the uploaded data
        GLuint m_vertexbuffer;
        GLuint m_uvbuffer;
        GLuint m_normalbuffer;
        GLuint m_tangentbuffer;
        GLuint m_bitangentbuffer;
    };
}