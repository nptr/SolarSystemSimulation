#include "../gl_core_3_3.h"

#include "Framebuffer.h"

#include <assert.h>

namespace jge
{
	Framebuffer FramebufferTools::CreateFramebuffer(GLuint width, GLuint height, bool depth, int samples)
	{
		Framebuffer fb;
		GLuint texType = (samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

		// Create sRGBA8 2D texture, 24 bit depth texture
		glGenTextures(1, &fb.colorTex);
		glBindTexture(texType, fb.colorTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (samples > 1) {
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_SRGB8_ALPHA8, width, height, GL_TRUE);
		}
		else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		}

		// Create framebuffer
		glGenFramebuffers(1, &fb.handle);
		glBindFramebuffer(GL_FRAMEBUFFER, fb.handle);

		// Attach color buffer to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texType, fb.colorTex, 0);

		if (depth)
		{
			// Create depth buffer
			glGenRenderbuffers(1, &fb.depthTex);
			glBindRenderbuffer(GL_RENDERBUFFER, fb.depthTex);

			if (samples > 1) {
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, width, height);
			}
			else {
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
			}

			// Attach depth buffer to FBO
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb.depthTex);
		}
			
		GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		assert(result == GL_FRAMEBUFFER_COMPLETE && "Framebuffer is not complete.\n");

		return fb;
	}

	void FramebufferTools::DeleteFramebuffer(Framebuffer& fb)
	{
		glDeleteTextures(1, &fb.colorTex);
		glDeleteRenderbuffers(1, &fb.depthTex);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fb.handle);
	}

	void FramebufferTools::DeleteFramebufferCube(CubeFramebuffer& fb)
	{
		glDeleteTextures(1, &fb.colorTex);
		glDeleteRenderbuffers(1, &fb.depthTex);
		glDeleteFramebuffers(6, (GLuint*)&fb.handle);
	}

	GLuint FramebufferTools::MakeCubeTexture_Color(GLsizei size)
	{
		GLuint cube;
		glGenTextures(1, &cube);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, 0);

		return cube;
	}


	GLuint FramebufferTools::MakeCubeTexture_Depth(GLsizei size)
	{
		GLuint cube;
		glGenTextures(1, &cube);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

		return cube;
	}


	CubeFramebuffer FramebufferTools::MakeFramebufferCube(GLuint colorTexture, GLuint depthTexture)
	{
		CubeFramebuffer fb;
		fb.colorTex = colorTexture;
		fb.depthTex = depthTexture;

		glGenFramebuffers(6, fb.handle);

		for (int i = 0; i < 6; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fb.handle[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, colorTexture, 0);
				
			if (depthTexture) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, depthTexture, 0);
			}

			GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			assert(result == GL_FRAMEBUFFER_COMPLETE && "Framebuffer is not complete.\n");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return fb;
	}
}