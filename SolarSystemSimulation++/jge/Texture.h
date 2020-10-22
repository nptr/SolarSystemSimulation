#pragma once

#include "../gl_core_3_3.h"

namespace jge
{
	class Texture
	{
	public:
		static GLuint LoadTexture(const char* file);
		static GLuint LoadTexture(const char* file, GLuint wrapMode, bool srgbInternal);

		static GLuint LoadCubemap(const char* posx,
			const char* negx,
			const char* posy,
			const char* negy,
			const char* posz,
			const char* negz);

	private:
		static GLuint LoadCubemapInternal(const char** faces);
	};


	enum BlendMode
	{
		NORMAL = 0,
		SCREEN = 1,
	};
}