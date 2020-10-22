#pragma once

#include "../gl_core_3_3.h"

namespace jge
{
	struct Framebuffer
	{
		GLuint colorTex;
		GLuint depthTex;
		GLuint handle;
	};

	struct CubeFramebuffer
	{
		GLuint colorTex;
		GLuint depthTex;
		GLuint handle[6];
	};

	class FramebufferTools
	{
	public:

        static Framebuffer CreateFramebuffer(GLuint width, GLuint height, bool depth, int samples = 1);
        static void DeleteFramebuffer(Framebuffer& fb);
        static void DeleteFramebufferCube(CubeFramebuffer& fb);
        static GLuint MakeCubeTexture_Color(GLsizei size);
        static GLuint MakeCubeTexture_Depth(GLsizei size);
        static CubeFramebuffer MakeFramebufferCube(GLuint colorTexture, GLuint depthTexture = 0);
	};

}