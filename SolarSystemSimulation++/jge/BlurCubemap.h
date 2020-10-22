#pragma once
#include "../gl_core_3_3.h"
#include "Mesh.h"
#include "Framebuffer.h"
#include "ShaderProgram.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

// This file contain everything necessary to blur a cubemap
// It is however ugly code, ignores the architecture of jge
// and currently not used.

namespace RenderTechniques
{
	namespace BlurCubemap
	{
		const GLfloat positiveZ[] =
		{
			1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f
		};

		const GLfloat negativeZ[] =
		{
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f
		};

		const GLfloat positiveX[] =
		{
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, -1.0f
		};

		const GLfloat negativeX[] =
		{
			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, -1.0f
		};

		const GLfloat positiveY[] =
		{
			-1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f
		};

		const GLfloat negativeY[] =
		{
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f
		};

		// World-View-Projection Matrix uniforms
		const char UF_VIEW_MATRIX[] = "view";
		const char UF_PROJECTION_MATRIX[] = "proj";
		const char UF_MODEL_MATRIX[] = "model";

		// Blurshader options
		const char UF_BLURTEX_SAMPLER[] = "textureSource";
		const char UF_BLURSCALE[] = "ScaleU";

		class BlurCubemap
		{
		public:
			BlurCubemap(int size)
				: blurTextureSize(size)
			{
				GLuint blurVShader = LoadShader(GL_VERTEX_SHADER, "shader\\basic.vert");
				GLuint blurFShader = LoadShader(GL_FRAGMENT_SHADER, "shader\\gaussBlur3D.frag");
				blurShader.Create(blurVShader, blurFShader);

				GLuint blurTex = jge::FramebufferTools::MakeCubeTexture_Color(blurTextureSize);
				blurFBOs = jge::FramebufferTools::MakeFramebufferCube(blurTex);

				cubeFaces[0] = CreateCubeFaceMesh(positiveX);
				cubeFaces[1] = CreateCubeFaceMesh(negativeX);
				cubeFaces[2] = CreateCubeFaceMesh(positiveY);
				cubeFaces[3] = CreateCubeFaceMesh(negativeY);
				cubeFaces[4] = CreateCubeFaceMesh(positiveZ);
				cubeFaces[5] = CreateCubeFaceMesh(negativeZ);

				blurScaleH[0] = glm::vec3(0.0f, 0.0f, 0.00195312f);
				blurScaleH[1] = glm::vec3(0.0f, 0.0f, 0.00195312f);
				blurScaleH[2] = glm::vec3(0.00195312f, 0.0f, 0.0f);
				blurScaleH[3] = glm::vec3(0.00195312f, 0.0f, 0.0f);
				blurScaleH[4] = glm::vec3(0.00195312f, 0.0f, 0.0f);
				blurScaleH[5] = glm::vec3(0.00195312f, 0.0f, 0.0f);

				blurScaleV[0] = glm::vec3(0.0f, 0.00195312f, 0.0f);
				blurScaleV[1] = glm::vec3(0.0f, 0.00195312f, 0.0f);
				blurScaleV[2] = glm::vec3(0.0f, 0.0f, 0.00195312f);
				blurScaleV[3] = glm::vec3(0.0f, 0.0f, 0.00195312f);
				blurScaleV[4] = glm::vec3(0.0f, 0.00195312f, 0.0f);
				blurScaleV[5] = glm::vec3(0.0f, 0.00195312f, 0.0f);
			}

			~BlurCubemap();

			void BlurCubeMap(jge::CubeFramebuffer cube)
			{
				// Bind blur shader
				blurShader.UseProgram();
				blurShader.UpdateUniform(UF_BLURTEX_SAMPLER, 0);

				// Bind shadowcube of light 1, i won't support more lights for now
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cube.colorTex);

				// Adjust Viewport to blur texture size
				glViewport(0, 0, blurTextureSize, blurTextureSize);

				// set projection to ortho (2D)
				glm::mat4 projMat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
				blurShader.UpdateUniform(UF_PROJECTION_MATRIX, projMat);
				blurShader.UpdateUniform(UF_VIEW_MATRIX, glm::lookAt(glm::vec3(0, 0, 0.9f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
				blurShader.UpdateUniform(UF_MODEL_MATRIX, glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f)));

				// for all sides of cube
				for (int i = 0; i < 6; ++i)
				{
					// Set vertical blur
					blurShader.UpdateUniform(UF_BLURSCALE, blurScaleV[i]);

					// Bind side of the blurbuffer as rendertarget and render to it.
					glBindFramebuffer(GL_FRAMEBUFFER, blurFBOs.handle[i]);
					cubeFaces[i]->Draw();
				}

				// Again, now blur horizontally
				glBindTexture(GL_TEXTURE_CUBE_MAP, blurFBOs.colorTex);
				glViewport(0, 0, blurTextureSize, blurTextureSize);

				for (int i = 0; i < 6; ++i)
				{
					blurShader.UpdateUniform(UF_BLURSCALE, blurScaleH[i]);
					glBindFramebuffer(GL_FRAMEBUFFER, cube.handle[i]);
					cubeFaces[i]->Draw();
				}
			}

			void DebugDrawShadowMapFace(jge::CubeFramebuffer cube, int face)
			{
				// Bind shader
				blurShader.UseProgram();
				blurShader.UpdateUniform(UF_BLURTEX_SAMPLER, 0);

				// Bind oldcubemap (the full) - only for the first light for now
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, blurFBOs.colorTex);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glViewport(0, 0, blurTextureSize, blurTextureSize);
				glClear(GL_COLOR_BUFFER_BIT);

				// set projection to ortho
				glm::mat4 projMat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
				blurShader.UpdateUniform(UF_PROJECTION_MATRIX, projMat);
				blurShader.UpdateUniform(UF_VIEW_MATRIX, glm::lookAt(glm::vec3(0, 0, 0.9f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
				blurShader.UpdateUniform(UF_MODEL_MATRIX, glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f)));

				cubeFaces[face]->Draw();
			}

		private:
			void* ReadEntireFile(const char* filename, int* length)
			{
				void* buffer;
				FILE* f = fopen(filename, "r");

				if (!f)
				{
					char buffer[256];
					sprintf(buffer, "Cannot open or find %s", filename);
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
					return jge::ShaderProgram::CreateShader(type, data, length);
				}
				catch (const std::runtime_error& ex)
				{
					throw std::runtime_error(std::string(path) + std::string("\r\n") + std::string(ex.what()));
				}
			}

			jge::Mesh* CreateCubeFaceMesh(const GLfloat* normals)
			{
				jge::Mesh* quad = new jge::Mesh();
				auto verts = quad->GetVertices();
				verts->push_back(glm::vec3(0.0f, 0.0f, 0.0f));
				verts->push_back(glm::vec3(1.0f, 0.0f, 0.0f));
				verts->push_back(glm::vec3(1.0f, 1.0f, 0.0f));
				verts->push_back(glm::vec3(0.0f, 1.0f, 0.0f));

				auto uvs = quad->GetTexCoords3D();
				uvs->push_back(glm::vec3(normals[0], normals[1], normals[2]));
				uvs->push_back(glm::vec3(normals[3], normals[4], normals[5]));
				uvs->push_back(glm::vec3(normals[6], normals[7], normals[8]));
				uvs->push_back(glm::vec3(normals[9], normals[10], normals[11]));

				quad->Create(GL_TRIANGLE_FAN, GL_STATIC_DRAW);
				return quad;
			}

			int blurTextureSize;

			jge::ShaderProgram blurShader;

			jge::CubeFramebuffer blurFBOs;
			jge::Mesh* cubeFaces[6];
			glm::vec3 blurScaleH[6];
			glm::vec3 blurScaleV[6];
		};
	}
}