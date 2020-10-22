#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <algorithm>    // std::sort

namespace jge
{
	class TransparencySorter
	{
	public:
		static void SortTrianglesByDistanceToCamera(const glm::vec3& cameraPos, Mesh* mesh, const glm::mat4& modelMatrix)
		{
			std::vector<glm::vec3>* verts = mesh->GetVertices();
			std::vector<glm::vec2>* uvs = mesh->GetTexCoords2D();
			std::vector<glm::vec3>* normals = mesh->GetNormals();

			std::vector<Triangle> triangles;

			for (unsigned int i = 0; i < verts->size(); i += 3)
			{
				Triangle t = Triangle((*verts)[i], (*verts)[i + 1], (*verts)[i + 2], (*uvs)[i], (*uvs)[i + 1], (*uvs)[i + 2], (*normals)[i], (*normals)[i + 1], (*normals)[i + 2], modelMatrix);
				triangles.push_back(t);
			}

			std::sort(triangles.begin(), triangles.end(), CompareTriangleDistanceStruct(cameraPos));

			verts->clear();
			uvs->clear();
			normals->clear();
			for (unsigned int i = 0; i < triangles.size(); i++)
			{
				verts->push_back(triangles[i].verts[0]);
				verts->push_back(triangles[i].verts[1]);
				verts->push_back(triangles[i].verts[2]);

				uvs->push_back(triangles[i].uvs[0]);
				uvs->push_back(triangles[i].uvs[1]);
				uvs->push_back(triangles[i].uvs[2]);

				normals->push_back(triangles[i].normals[0]);
				normals->push_back(triangles[i].normals[1]);
				normals->push_back(triangles[i].normals[2]);
			}
		}

	private:

		struct Triangle
		{
			Triangle(glm::vec3 _a, glm::vec3 _b, glm::vec3 _c, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, glm::vec3 n1, glm::vec3 n2, glm::vec3 n3, glm::mat4 mm)
			{
				verts[0] = _a;
				verts[1] = _b;
				verts[2] = _c;
				s = CalcCentroid(glm::vec3(mm*glm::vec4(_a, 1.0f)), glm::vec3(mm*glm::vec4(_b, 1.0f)), glm::vec3(mm*glm::vec4(_c, 1.0f)));

				uvs[0] = uv1;
				uvs[1] = uv2;
				uvs[2] = uv3;

				normals[0] = n1;
				normals[1] = n2;
				normals[2] = n3;
			}

			glm::vec3 s;
			glm::vec3 verts[3];
			glm::vec2 uvs[3];
			glm::vec3 normals[3];
		};

		static inline glm::vec3 CalcCentroid(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
		{
			return (v1 + v2 + v3) / glm::vec3(3, 3, 3);
		}

		struct CompareTriangleDistanceStruct
		{
			glm::vec3 camPos;
			CompareTriangleDistanceStruct(const glm::vec3& c)
			{
				camPos = c;
			}

			inline bool operator() (Triangle& v1, Triangle& v2)
			{
				float tempV1 = glm::length(camPos - v1.s);
				float tempV2 = glm::length(camPos - v2.s);

				return (tempV1 > tempV2);
			}
		};
	};
}
