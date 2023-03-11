#include "Mesh.h"

#include <cstdio>
#include <cstring>

using namespace std;
using namespace glm;

namespace jge
{
    Mesh::Mesh()
        : m_vertexArrayID(0)
        , m_drawType(0)
        , m_usageType(0)
        , m_vertices()
        , m_uvs()
        , m_uvws()
        , m_normals()
        , m_tangents()
        , m_bitangents()
        , m_vertexbuffer(0)
        , m_uvbuffer(0)
        , m_normalbuffer(0)
        , m_tangentbuffer(0)
        , m_bitangentbuffer(0)
    {
    }

    Mesh::~Mesh()
    {
        // We can safely delete everything - even if the object wasn't created
        glDeleteVertexArrays(1, &m_vertexArrayID);

        glDeleteBuffers(1, &m_vertexbuffer);
        glDeleteBuffers(1, &m_uvbuffer);
        glDeleteBuffers(1, &m_normalbuffer);
        glDeleteBuffers(1, &m_tangentbuffer);
        glDeleteBuffers(1, &m_bitangentbuffer);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
        glDisableVertexAttribArray(4);
        glBindVertexArray(0);
    }

    void Mesh::SetBufferData(GLuint bufferID, const vector<vec2>& data, GLenum usage)
    {
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec2), data.data(), usage);
    }

    void Mesh::SetBufferData(GLuint bufferID, const vector<vec3>& data, GLenum usage)
    {
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec3), data.data(), usage);
    }

    void Mesh::Attach(GLuint bufferID, int location, int size)
    {
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(
            location,						  // attribute
            size,                             // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );
    }

    void Mesh::Create(GLenum vertexType, GLenum usage, bool generateTangents)
    {
        // Create VAO
        glGenVertexArrays(1, &m_vertexArrayID);

        // Create buffers
        glGenBuffers(1, &m_vertexbuffer);
        glGenBuffers(1, &m_uvbuffer);
        glGenBuffers(1, &m_normalbuffer);
        glGenBuffers(1, &m_tangentbuffer);
        glGenBuffers(1, &m_bitangentbuffer);

        m_drawType = vertexType;
        m_usageType = usage;

        if (generateTangents)
            GenerateTangents();

        Update();
    }

    void Mesh::Update()
    {
        glBindVertexArray(m_vertexArrayID);

        // Bind vertexbuffer
        SetBufferData(m_vertexbuffer, m_vertices, m_usageType);
        Attach(m_vertexbuffer, 0, 3);

        // Bind normalbuffer
        if (m_normals.size() > 0)
        {
            SetBufferData(m_normalbuffer, m_normals, m_usageType);
            Attach(m_normalbuffer, 1, 3);
        }

        // Bind inout_tangent buffer
        if (m_tangents.size() > 0)
        {
            SetBufferData(m_tangentbuffer, m_tangents, m_usageType);
            Attach(m_tangentbuffer, 3, 3);
        }

        // Bind bi-inout_tangent buffer
        if (m_bitangents.size() > 0)
        {
            SetBufferData(m_bitangentbuffer, m_bitangents, m_usageType);
            Attach(m_bitangentbuffer, 4, 3);
        }

        // Bind uv buffer (2D)
        if (m_uvs.size() > 0 && m_uvs.size() >= m_uvws.size())
        {
            SetBufferData(m_uvbuffer, m_uvs, m_usageType);
            Attach(m_uvbuffer, 2, 2);
        }

        // Bind uv buffer (3D)
        if (m_uvws.size() > 0 && m_uvs.size() <= m_uvws.size())
        {
            SetBufferData(m_uvbuffer, m_uvws, m_usageType);
            Attach(m_uvbuffer, 2, 3);
        }
    }

    GLuint Mesh::GetVAO() const
    {
        return m_vertexArrayID;
    }

    void Mesh::Draw(GLenum mode) const
    {
        glBindVertexArray(m_vertexArrayID);
        glDrawArrays(mode, 0, static_cast<GLsizei>(m_vertices.size()));
    }

    void Mesh::Draw() const
    {
        Draw(m_drawType);
    }

    void Mesh::DrawNoBind() const
    {
        glDrawArrays(m_drawType, 0, static_cast<GLsizei>(m_vertices.size()));
    }

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
    void Mesh::GenerateTangents()
    {
        for (size_t i = 0; i < m_vertices.size(); i += 3)
        {
            // Edges of the triangle : postion delta
            glm::vec3& v0 = m_vertices[i + 0];
            glm::vec3 deltaPos1 = m_vertices[i + 1] - v0;
            glm::vec3 deltaPos2 = m_vertices[i + 2] - v0;

            // UV delta
            glm::vec2& uv0 = m_uvs[i + 0];
            glm::vec2 deltaUV1 = m_uvs[i + 1] - uv0;
            glm::vec2 deltaUV2 = m_uvs[i + 2] - uv0;

            float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
            glm::vec3 inout_tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
            glm::vec3 inout_bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

            m_tangents.push_back(inout_tangent);
            m_tangents.push_back(inout_tangent);
            m_tangents.push_back(inout_tangent);

            m_bitangents.push_back(inout_bitangent);
            m_bitangents.push_back(inout_bitangent);
            m_bitangents.push_back(inout_bitangent);
        }

        // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#going-further
        for (unsigned int i = 0; i < m_vertices.size(); i += 1)
        {
            glm::vec3& n = (m_normals)[i];
            glm::vec3& t = (m_tangents)[i];
            //glm::vec3 & b = (m_bitangents)[i];

            // Gram-Schmidt orthogonalize
            t = glm::normalize(t - n * glm::dot(n, t));

            // Calculate handedness
            /*if (glm::dot(glm::cross(n, t), b) < 0.0f){
                t = t * -1.0f;
            }*/
        }
    }

    void Mesh::SaveBufferToFile(const char* objFile)
    {
        FILE* file = fopen(objFile, "w");

        fprintf(file, "const vertices = [\n");
        std::vector<glm::vec3>::iterator it = m_vertices.begin();
        for (; it != m_vertices.end(); ++it)
        {
            glm::vec3& v = *it;
            fprintf(file, "\t%.4f, %.4f, %.4f,\n", v.x, v.y, v.z);
        }
        fprintf(file, "];\n\n");

        fprintf(file, "const uvs = [\n");
        std::vector<glm::vec2>::iterator it2 = m_uvs.begin();
        for (; it2 != m_uvs.end(); ++it2)
        {
            glm::vec2& v = *it2;
            fprintf(file, "\t%.4f, %.4f,\n", v.x, v.y);
        }
        fprintf(file, "];\n\n");

        fprintf(file, "const normals = [\n");
        std::vector<glm::vec3>::iterator it3 = m_normals.begin();
        for (; it3 != m_normals.end(); ++it3)
        {
            glm::vec3& v = *it3;
            fprintf(file, "\t%.4f, %.4f, %.4f,\n", v.x, v.y, v.z);
        }
        fprintf(file, "];\n\n");

        fprintf(file, "const tangents = [\n");
        std::vector<glm::vec3>::iterator it4 = m_tangents.begin();
        for (; it4 != m_tangents.end(); ++it4)
        {
            glm::vec3& v = *it4;
            fprintf(file, "\t%.4f, %.4f, %.4f,\n", v.x, v.y, v.z);
        }
        fprintf(file, "];\n\n");

        fprintf(file, "const bitangents = [\n");
        std::vector<glm::vec3>::iterator it5 = m_bitangents.begin();
        for (; it5 != m_bitangents.end(); ++it5)
        {
            glm::vec3& v = *it5;
            fprintf(file, "\t%.4f, %.4f, %.4f,\n", v.x, v.y, v.z);
        }
        fprintf(file, "];\n\n");

        fclose(file);
    }

    // http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
    void Mesh::FromObjectFile(const char* path, bool generateTangents)
    {
        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
        std::vector<glm::vec3> temp_vertices;
        std::vector<glm::vec2> temp_uvs;
        std::vector<glm::vec3> temp_normals;

        FILE* file = fopen(path, "r");
        if (file == NULL) {
            fprintf(stdout, "Model::FromObjectFile: Cannot open file: %s", path);
            return;
        }

        char lineHeader[128];
        int res;

        while ((res = fscanf(file, "%s", lineHeader)) != EOF)
        {
            // vertex data
            if (strcmp(lineHeader, "v") == 0)
            {
                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                temp_vertices.push_back(vertex);
            }
            // texture coords
            else if (strcmp(lineHeader, "vt") == 0)
            {
                glm::vec2 uv;
                fscanf(file, "%f %f\n", &uv.x, &uv.y);
                uv.y = 1 - uv.y;						// OpenGL has a different view on things as blender..
                temp_uvs.push_back(uv);
            }
            // normals
            else if (strcmp(lineHeader, "vn") == 0)
            {
                glm::vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                temp_normals.push_back(normal);
            }
            // faces
            else if (strcmp(lineHeader, "f") == 0)
            {
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
                if (matches != 9) {
                    fprintf(stdout, "Model::FromObjectFile: Cannot read this format!");
                    return;
                }
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[0]);
                uvIndices.push_back(uvIndex[1]);
                uvIndices.push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            }
            else
            {
                // skip that line
                char stupidBuffer[1000];
                fgets(stupidBuffer, 1000, file);
            }
        }

        // For each vertex of each triangle
        for (unsigned int i = 0; i < vertexIndices.size(); i++)
        {
            // Put the attributes in buffers
            m_vertices.push_back(temp_vertices[vertexIndices[i] - 1]);
            m_uvs.push_back(temp_uvs[uvIndices[i] - 1]);
            m_normals.push_back(temp_normals[normalIndices[i] - 1]);
        }

        Create(GL_TRIANGLES, GL_STATIC_DRAW, generateTangents);
    }

    void Mesh::CreateCircle()
    {
        float circleVerts[] =
        {
            1.000000f, 0.000000f, 0.000000f,
            0.999848f, 0.000000f, 0.017452f,
            0.999391f, 0.000000f, 0.034899f,
            0.998630f, 0.000000f, 0.052336f,
            0.997564f, 0.000000f, 0.069756f,
            0.996195f, 0.000000f, 0.087156f,
            0.994522f, 0.000000f, 0.104528f,
            0.992546f, 0.000000f, 0.121869f,
            0.990268f, 0.000000f, 0.139173f,
            0.987688f, 0.000000f, 0.156434f,
            0.984808f, 0.000000f, 0.173648f,
            0.981627f, 0.000000f, 0.190809f,
            0.978148f, 0.000000f, 0.207912f,
            0.974370f, 0.000000f, 0.224951f,
            0.970296f, 0.000000f, 0.241922f,
            0.965926f, 0.000000f, 0.258819f,
            0.961262f, 0.000000f, 0.275637f,
            0.956305f, 0.000000f, 0.292371f,
            0.951057f, 0.000000f, 0.309017f,
            0.945519f, 0.000000f, 0.325568f,
            0.939693f, 0.000000f, 0.342020f,
            0.933581f, 0.000000f, 0.358368f,
            0.927184f, 0.000000f, 0.374606f,
            0.920505f, 0.000000f, 0.390731f,
            0.913546f, 0.000000f, 0.406736f,
            0.906308f, 0.000000f, 0.422618f,
            0.898794f, 0.000000f, 0.438371f,
            0.891007f, 0.000000f, 0.453990f,
            0.882948f, 0.000000f, 0.469471f,
            0.874620f, 0.000000f, 0.484809f,
            0.866026f, 0.000000f, 0.500000f,
            0.857168f, 0.000000f, 0.515038f,
            0.848048f, 0.000000f, 0.529919f,
            0.838671f, 0.000000f, 0.544639f,
            0.829038f, 0.000000f, 0.559193f,
            0.819152f, 0.000000f, 0.573576f,
            0.809017f, 0.000000f, 0.587785f,
            0.798636f, 0.000000f, 0.601815f,
            0.788011f, 0.000000f, 0.615661f,
            0.777146f, 0.000000f, 0.629320f,
            0.766045f, 0.000000f, 0.642787f,
            0.754710f, 0.000000f, 0.656059f,
            0.743145f, 0.000000f, 0.669130f,
            0.731354f, 0.000000f, 0.681998f,
            0.719340f, 0.000000f, 0.694658f,
            0.707107f, 0.000000f, 0.707106f,
            0.694659f, 0.000000f, 0.719339f,
            0.681999f, 0.000000f, 0.731353f,
            0.669131f, 0.000000f, 0.743144f,
            0.656060f, 0.000000f, 0.754709f,
            0.642788f, 0.000000f, 0.766044f,
            0.629321f, 0.000000f, 0.777146f,
            0.615662f, 0.000000f, 0.788010f,
            0.601816f, 0.000000f, 0.798635f,
            0.587786f, 0.000000f, 0.809017f,
            0.573577f, 0.000000f, 0.819152f,
            0.559194f, 0.000000f, 0.829037f,
            0.544640f, 0.000000f, 0.838670f,
            0.529920f, 0.000000f, 0.848048f,
            0.515039f, 0.000000f, 0.857167f,
            0.500001f, 0.000000f, 0.866025f,
            0.484810f, 0.000000f, 0.874619f,
            0.469472f, 0.000000f, 0.882947f,
            0.453991f, 0.000000f, 0.891006f,
            0.438372f, 0.000000f, 0.898794f,
            0.422619f, 0.000000f, 0.906307f,
            0.406737f, 0.000000f, 0.913545f,
            0.390732f, 0.000000f, 0.920504f,
            0.374607f, 0.000000f, 0.927183f,
            0.358369f, 0.000000f, 0.933580f,
            0.342021f, 0.000000f, 0.939692f,
            0.325569f, 0.000000f, 0.945518f,
            0.309018f, 0.000000f, 0.951056f,
            0.292373f, 0.000000f, 0.956304f,
            0.275638f, 0.000000f, 0.961261f,
            0.258820f, 0.000000f, 0.965926f,
            0.241923f, 0.000000f, 0.970295f,
            0.224952f, 0.000000f, 0.974370f,
            0.207913f, 0.000000f, 0.978147f,
            0.190810f, 0.000000f, 0.981627f,
            0.173649f, 0.000000f, 0.984808f,
            0.156436f, 0.000000f, 0.987688f,
            0.139174f, 0.000000f, 0.990268f,
            0.121871f, 0.000000f, 0.992546f,
            0.104530f, 0.000000f, 0.994522f,
            0.087157f, 0.000000f, 0.996195f,
            0.069758f, 0.000000f, 0.997564f,
            0.052337f, 0.000000f, 0.998629f,
            0.034901f, 0.000000f, 0.999391f,
            0.017454f, 0.000000f, 0.999848f,
            0.000001f, 0.000000f, 1.000000f,
            -0.017451f, 0.000000f, 0.999848f,
            -0.034898f, 0.000000f, 0.999391f,
            -0.052335f, 0.000000f, 0.998630f,
            -0.069755f, 0.000000f, 0.997564f,
            -0.087154f, 0.000000f, 0.996195f,
            -0.104527f, 0.000000f, 0.994522f,
            -0.121868f, 0.000000f, 0.992546f,
            -0.139172f, 0.000000f, 0.990268f,
            -0.156433f, 0.000000f, 0.987689f,
            -0.173647f, 0.000000f, 0.984808f,
            -0.190808f, 0.000000f, 0.981627f,
            -0.207910f, 0.000000f, 0.978148f,
            -0.224950f, 0.000000f, 0.974370f,
            -0.241920f, 0.000000f, 0.970296f,
            -0.258818f, 0.000000f, 0.965926f,
            -0.275636f, 0.000000f, 0.961262f,
            -0.292370f, 0.000000f, 0.956305f,
            -0.309016f, 0.000000f, 0.951057f,
            -0.325567f, 0.000000f, 0.945519f,
            -0.342019f, 0.000000f, 0.939693f,
            -0.358366f, 0.000000f, 0.933581f,
            -0.374605f, 0.000000f, 0.927184f,
            -0.390730f, 0.000000f, 0.920505f,
            -0.406735f, 0.000000f, 0.913546f,
            -0.422617f, 0.000000f, 0.906308f,
            -0.438370f, 0.000000f, 0.898795f,
            -0.453989f, 0.000000f, 0.891007f,
            -0.469470f, 0.000000f, 0.882948f,
            -0.484808f, 0.000000f, 0.874621f,
            -0.499999f, 0.000000f, 0.866026f,
            -0.515037f, 0.000000f, 0.857168f,
            -0.529918f, 0.000000f, 0.848049f,
            -0.544638f, 0.000000f, 0.838672f,
            -0.559191f, 0.000000f, 0.829039f,
            -0.573575f, 0.000000f, 0.819153f,
            -0.587784f, 0.000000f, 0.809018f,
            -0.601814f, 0.000000f, 0.798637f,
            -0.615660f, 0.000000f, 0.788012f,
            -0.629319f, 0.000000f, 0.777147f,
            -0.642786f, 0.000000f, 0.766046f,
            -0.656058f, 0.000000f, 0.754711f,
            -0.669129f, 0.000000f, 0.743146f,
            -0.681997f, 0.000000f, 0.731355f,
            -0.694657f, 0.000000f, 0.719341f,
            -0.707105f, 0.000000f, 0.707108f,
            -0.719338f, 0.000000f, 0.694660f,
            -0.731352f, 0.000000f, 0.682000f,
            -0.743144f, 0.000000f, 0.669132f,
            -0.754708f, 0.000000f, 0.656061f,
            -0.766043f, 0.000000f, 0.642789f,
            -0.777145f, 0.000000f, 0.629322f,
            -0.788010f, 0.000000f, 0.615663f,
            -0.798634f, 0.000000f, 0.601817f,
            -0.809016f, 0.000000f, 0.587787f,
            -0.819151f, 0.000000f, 0.573578f,
            -0.829036f, 0.000000f, 0.559195f,
            -0.838669f, 0.000000f, 0.544641f,
            -0.848047f, 0.000000f, 0.529921f,
            -0.857166f, 0.000000f, 0.515040f,
            -0.866024f, 0.000000f, 0.500002f,
            -0.874619f, 0.000000f, 0.484811f,
            -0.882947f, 0.000000f, 0.469473f,
            -0.891006f, 0.000000f, 0.453992f,
            -0.898793f, 0.000000f, 0.438373f,
            -0.906307f, 0.000000f, 0.422620f,
            -0.913545f, 0.000000f, 0.406739f,
            -0.920504f, 0.000000f, 0.390733f,
            -0.927183f, 0.000000f, 0.374609f,
            -0.933580f, 0.000000f, 0.358370f,
            -0.939692f, 0.000000f, 0.342022f,
            -0.945518f, 0.000000f, 0.325570f,
            -0.951056f, 0.000000f, 0.309019f,
            -0.956304f, 0.000000f, 0.292374f,
            -0.961261f, 0.000000f, 0.275640f,
            -0.965925f, 0.000000f, 0.258821f,
            -0.970295f, 0.000000f, 0.241924f,
            -0.974370f, 0.000000f, 0.224953f,
            -0.978147f, 0.000000f, 0.207914f,
            -0.981627f, 0.000000f, 0.190811f,
            -0.984807f, 0.000000f, 0.173651f,
            -0.987688f, 0.000000f, 0.156437f,
            -0.990268f, 0.000000f, 0.139175f,
            -0.992546f, 0.000000f, 0.121872f,
            -0.994522f, 0.000000f, 0.104531f,
            -0.996194f, 0.000000f, 0.087158f,
            -0.997564f, 0.000000f, 0.069759f,
            -0.998629f, 0.000000f, 0.052338f,
            -0.999391f, 0.000000f, 0.034902f,
            -0.999848f, 0.000000f, 0.017455f,
            -1.000000f, 0.000000f, 0.000003f,
            -0.999848f, 0.000000f, -0.017450f,
            -0.999391f, 0.000000f, -0.034897f,
            -0.998630f, 0.000000f, -0.052333f,
            -0.997564f, 0.000000f, -0.069754f,
            -0.996195f, 0.000000f, -0.087153f,
            -0.994522f, 0.000000f, -0.104526f,
            -0.992546f, 0.000000f, -0.121867f,
            -0.990268f, 0.000000f, -0.139170f,
            -0.987689f, 0.000000f, -0.156432f,
            -0.984808f, 0.000000f, -0.173646f,
            -0.981628f, 0.000000f, -0.190806f,
            -0.978148f, 0.000000f, -0.207909f,
            -0.974371f, 0.000000f, -0.224948f,
            -0.970296f, 0.000000f, -0.241919f,
            -0.965927f, 0.000000f, -0.258816f,
            -0.961262f, 0.000000f, -0.275635f,
            -0.956306f, 0.000000f, -0.292369f,
            -0.951057f, 0.000000f, -0.309014f,
            -0.945519f, 0.000000f, -0.325566f,
            -0.939694f, 0.000000f, -0.342017f,
            -0.933581f, 0.000000f, -0.358365f,
            -0.927185f, 0.000000f, -0.374604f,
            -0.920506f, 0.000000f, -0.390728f,
            -0.913547f, 0.000000f, -0.406734f,
            -0.906309f, 0.000000f, -0.422616f,
            -0.898795f, 0.000000f, -0.438369f,
            -0.891008f, 0.000000f, -0.453988f,
            -0.882949f, 0.000000f, -0.469469f,
            -0.874621f, 0.000000f, -0.484807f,
            -0.866027f, 0.000000f, -0.499997f,
            -0.857169f, 0.000000f, -0.515036f,
            -0.848050f, 0.000000f, -0.529917f,
            -0.838672f, 0.000000f, -0.544637f,
            -0.829039f, 0.000000f, -0.559190f,
            -0.819154f, 0.000000f, -0.573574f,
            -0.809019f, 0.000000f, -0.587783f,
            -0.798637f, 0.000000f, -0.601813f,
            -0.788013f, 0.000000f, -0.615659f,
            -0.777148f, 0.000000f, -0.629318f,
            -0.766046f, 0.000000f, -0.642785f,
            -0.754712f, 0.000000f, -0.656057f,
            -0.743147f, 0.000000f, -0.669128f,
            -0.731356f, 0.000000f, -0.681996f,
            -0.719342f, 0.000000f, -0.694656f,
            -0.707109f, 0.000000f, -0.707105f,
            -0.694661f, 0.000000f, -0.719338f,
            -0.682001f, 0.000000f, -0.731352f,
            -0.669133f, 0.000000f, -0.743143f,
            -0.656061f, 0.000000f, -0.754707f,
            -0.642790f, 0.000000f, -0.766042f,
            -0.629323f, 0.000000f, -0.777144f,
            -0.615664f, 0.000000f, -0.788009f,
            -0.601818f, 0.000000f, -0.798634f,
            -0.587788f, 0.000000f, -0.809015f,
            -0.573579f, 0.000000f, -0.819150f,
            -0.559196f, 0.000000f, -0.829036f,
            -0.544642f, 0.000000f, -0.838669f,
            -0.529922f, 0.000000f, -0.848046f,
            -0.515041f, 0.000000f, -0.857166f,
            -0.500003f, 0.000000f, -0.866024f,
            -0.484813f, 0.000000f, -0.874618f,
            -0.469475f, 0.000000f, -0.882946f,
            -0.453994f, 0.000000f, -0.891005f,
            -0.438374f, 0.000000f, -0.898793f,
            -0.422621f, 0.000000f, -0.906306f,
            -0.406740f, 0.000000f, -0.913544f,
            -0.390734f, 0.000000f, -0.920503f,
            -0.374610f, 0.000000f, -0.927183f,
            -0.358371f, 0.000000f, -0.933579f,
            -0.342023f, 0.000000f, -0.939691f,
            -0.325571f, 0.000000f, -0.945517f,
            -0.309020f, 0.000000f, -0.951055f,
            -0.292375f, 0.000000f, -0.956304f,
            -0.275641f, 0.000000f, -0.961261f,
            -0.258823f, 0.000000f, -0.965925f,
            -0.241925f, 0.000000f, -0.970295f,
            -0.224955f, 0.000000f, -0.974369f,
            -0.207915f, 0.000000f, -0.978147f,
            -0.190813f, 0.000000f, -0.981626f,
            -0.173652f, 0.000000f, -0.984807f,
            -0.156438f, 0.000000f, -0.987688f,
            -0.139177f, 0.000000f, -0.990268f,
            -0.121873f, 0.000000f, -0.992546f,
            -0.104532f, 0.000000f, -0.994522f,
            -0.087159f, 0.000000f, -0.996194f,
            -0.069760f, 0.000000f, -0.997564f,
            -0.052340f, 0.000000f, -0.998629f,
            -0.034903f, 0.000000f, -0.999391f,
            -0.017456f, 0.000000f, -0.999848f,
            -0.000004f, 0.000000f, -1.000000f,
            0.017449f, 0.000000f, -0.999848f,
            0.034896f, 0.000000f, -0.999391f,
            0.052332f, 0.000000f, -0.998630f,
            0.069753f, 0.000000f, -0.997564f,
            0.087152f, 0.000000f, -0.996195f,
            0.104525f, 0.000000f, -0.994522f,
            0.121865f, 0.000000f, -0.992547f,
            0.139169f, 0.000000f, -0.990269f,
            0.156431f, 0.000000f, -0.987689f,
            0.173644f, 0.000000f, -0.984808f,
            0.190805f, 0.000000f, -0.981628f,
            0.207908f, 0.000000f, -0.978148f,
            0.224947f, 0.000000f, -0.974371f,
            0.241918f, 0.000000f, -0.970297f,
            0.258815f, 0.000000f, -0.965927f,
            0.275633f, 0.000000f, -0.961263f,
            0.292368f, 0.000000f, -0.956306f,
            0.309013f, 0.000000f, -0.951058f,
            0.325564f, 0.000000f, -0.945520f,
            0.342016f, 0.000000f, -0.939694f,
            0.358364f, 0.000000f, -0.933582f,
            0.374603f, 0.000000f, -0.927185f,
            0.390727f, 0.000000f, -0.920506f,
            0.406733f, 0.000000f, -0.913547f,
            0.422614f, 0.000000f, -0.906310f,
            0.438367f, 0.000000f, -0.898796f,
            0.453987f, 0.000000f, -0.891008f,
            0.469468f, 0.000000f, -0.882950f,
            0.484806f, 0.000000f, -0.874622f,
            0.499996f, 0.000000f, -0.866028f,
            0.515034f, 0.000000f, -0.857169f,
            0.529916f, 0.000000f, -0.848050f,
            0.544635f, 0.000000f, -0.838673f,
            0.559189f, 0.000000f, -0.829040f,
            0.573573f, 0.000000f, -0.819155f,
            0.587782f, 0.000000f, -0.809020f,
            0.601812f, 0.000000f, -0.798638f,
            0.615658f, 0.000000f, -0.788013f,
            0.629317f, 0.000000f, -0.777149f,
            0.642784f, 0.000000f, -0.766047f,
            0.656056f, 0.000000f, -0.754712f,
            0.669127f, 0.000000f, -0.743148f,
            0.681995f, 0.000000f, -0.731357f,
            0.694655f, 0.000000f, -0.719343f,
            0.707104f, 0.000000f, -0.707110f,
            0.719337f, 0.000000f, -0.694662f,
            0.731351f, 0.000000f, -0.682002f,
            0.743142f, 0.000000f, -0.669134f,
            0.754707f, 0.000000f, -0.656062f,
            0.766042f, 0.000000f, -0.642791f,
            0.777143f, 0.000000f, -0.629324f,
            0.788008f, 0.000000f, -0.615665f,
            0.798633f, 0.000000f, -0.601819f,
            0.809014f, 0.000000f, -0.587789f,
            0.819149f, 0.000000f, -0.573580f,
            0.829035f, 0.000000f, -0.559197f,
            0.838668f, 0.000000f, -0.544643f,
            0.848046f, 0.000000f, -0.529923f,
            0.857165f, 0.000000f, -0.515042f,
            0.866023f, 0.000000f, -0.500004f,
            0.874617f, 0.000000f, -0.484814f,
            0.882945f, 0.000000f, -0.469476f,
            0.891004f, 0.000000f, -0.453995f,
            0.898792f, 0.000000f, -0.438375f,
            0.906306f, 0.000000f, -0.422623f,
            0.913544f, 0.000000f, -0.406741f,
            0.920503f, 0.000000f, -0.390735f,
            0.927182f, 0.000000f, -0.374611f,
            0.933579f, 0.000000f, -0.358372f,
            0.939691f, 0.000000f, -0.342025f,
            0.945517f, 0.000000f, -0.325573f,
            0.951055f, 0.000000f, -0.309022f,
            0.956303f, 0.000000f, -0.292376f,
            0.961260f, 0.000000f, -0.275642f,
            0.965925f, 0.000000f, -0.258824f,
            0.970295f, 0.000000f, -0.241927f,
            0.974369f, 0.000000f, -0.224956f,
            0.978147f, 0.000000f, -0.207916f,
            0.981626f, 0.000000f, -0.190814f,
            0.984807f, 0.000000f, -0.173653f,
            0.987688f, 0.000000f, -0.156439f,
            0.990267f, 0.000000f, -0.139178f,
            0.992546f, 0.000000f, -0.121874f,
            0.994521f, 0.000000f, -0.104533f,
            0.996194f, 0.000000f, -0.087161f,
            0.997564f, 0.000000f, -0.069761f,
            0.998629f, 0.000000f, -0.052341f,
            0.999391f, 0.000000f, -0.034905f,
            0.999848f, 0.000000f, -0.017457f
        };

        m_vertices.resize(0);
        const size_t circleVertsSize = sizeof(circleVerts) / sizeof(circleVerts[0]);
        m_vertices.insert(m_vertices.begin(), reinterpret_cast<vec3*>(circleVerts), reinterpret_cast<vec3*>(circleVerts + circleVertsSize));
        Create(GL_LINE_LOOP, GL_STATIC_DRAW);
    }

    void Mesh::CreateQuad()
    {
        float quadVerts[] =
        {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f
        };

        float texCoords[] =
        {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        m_vertices.resize(0);
        const size_t quadVertsSize = sizeof(quadVerts) / sizeof(quadVerts[0]);
        m_vertices.insert(m_vertices.begin(), reinterpret_cast<vec3*>(quadVerts), reinterpret_cast<vec3*>(quadVerts + quadVertsSize));

        m_uvs.resize(0);
        const size_t uvSize = sizeof(texCoords) / sizeof(texCoords[0]);
        m_uvs.insert(m_uvs.begin(), reinterpret_cast<vec2*>(texCoords), reinterpret_cast<vec2*>(texCoords + uvSize));

        Create(GL_TRIANGLE_FAN, GL_STATIC_DRAW);
    }
}