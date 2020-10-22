#pragma once

#include "../gl_core_3_3.h"

///
/// Helper class for the creation of cubemaps
///
class CubeMapTools
{
public:
	static GLuint MakeCubeTexture_Color(GLsizei size);
	static GLuint MakeCubeTexture_Depth(GLsizei size);
	static void MakeFramebufferCube(GLuint* cubeFBOs, GLuint colorTexture);
	static void MakeFramebufferCube(GLuint* cubeFBOs, GLuint colorTexture, GLuint depthTexture);
};
