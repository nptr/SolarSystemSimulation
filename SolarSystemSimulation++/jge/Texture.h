#pragma once

#include "../gl_core_3_3.h"
#include <string>

namespace jge
{
    enum class BlendMode
    {
        NORMAL = 0,
        SCREEN = 1,
    };

    class Texture
    {
    public:
        static GLuint LoadTexture(const std::string& file);
        static GLuint LoadTexture(const std::string& file, GLuint wrapMode, bool srgbInternal);

        static GLuint LoadCubemap(const char* posx,
            const char* negx,
            const char* posy,
            const char* negy,
            const char* posz,
            const char* negz);

    private:
        static GLuint LoadCubemapInternal(const char** faces);
    };
}