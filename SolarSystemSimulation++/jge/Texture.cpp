#include "Texture.h"

// Place the impl here. Do this just once in the whole project
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

namespace jge
{
	GLuint Texture::LoadCubemap(const char* posx,
		const char* negx,
		const char* posy,
		const char* negy,
		const char* posz,
		const char* negz)
	{
		const char* fileArray[6] {posx, negx, posy, negy, posz, negz};
		return LoadCubemapInternal((const char**)&fileArray);
	}


	GLuint Texture::LoadCubemapInternal(const char** faces)
	{
		GLuint textureID;
		glGenTextures(1, &textureID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width;
		int height;
		int numComponents;
		stbi_uc* data;

		for (GLuint i = 0; i < 6; i++)
		{
			const char* fp = faces[i];
			data = stbi_load(faces[i], &width, &height, &numComponents, 3);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		return textureID;
	}

	GLuint Texture::LoadTexture(const char* file)
	{
		return LoadTexture(file, GL_REPEAT, true);
	}

	GLuint Texture::LoadTexture(const char* file,  GLuint wrapMode, bool srgbInternal)
	{
		int width;
		int height;
		int numComponents;

		stbi_uc*  data = stbi_load(file, &width, &height, &numComponents, 0);
		if (data != NULL)
		{
			GLuint textureID;
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);

			GLuint internalFormat;
			GLuint originalFormat;
			switch (numComponents)
			{
				case 1:
				{
					originalFormat = GL_RED;
					internalFormat = GL_RED;
				}break;
				case 2:
				{
					originalFormat = GL_RG;
					internalFormat = GL_RG;
				}break;
				case 3:
				{
					originalFormat = GL_RGB;
					if (srgbInternal)
						internalFormat = GL_SRGB;
					else internalFormat = GL_RGB;
				}break;
				case 4:
				{
					originalFormat = GL_RGBA;
					if (srgbInternal)
						internalFormat = GL_SRGB_ALPHA;
					else internalFormat = GL_RGBA;
				}break;
				default:
				{
					return -1;
				}
			}
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, originalFormat, GL_UNSIGNED_BYTE, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(data);
			return textureID;
		}

		return -1;
	}
}

