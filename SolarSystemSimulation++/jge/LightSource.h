#pragma once

#include <glm/glm.hpp>
#include "ShaderProgram.h"

namespace jge
{
    enum class LightComponent
    {
        AMBIENT = 0,
        DIFFUSE = 1,
        SPECULAR = 2
    };

    enum class LightType
    {
        SPOT = 0,
        DIRECTIONAL = 1,
        POINT = 2
    };

    class LightSource
    {
    public:
        static LightSource* CreatePointLight(const glm::vec3& pos);
        static LightSource* CreateSpotLight(const glm::vec3& pos, const glm::vec3& dir, float cutoff, float exponent);
        static LightSource* CreateDirectionalLight(const glm::vec3& dir);

    public:
        ~LightSource();

        void SetPosition(const glm::vec3& pos);
        glm::vec4 GetPosition() const;

        void SetColor(LightComponent type, float r, float g, float b);
        glm::vec4 GetColor(LightComponent type) const;

        void SetAttenuation(float constant, float linear, float quadratic);
        void GetAttenuation(float& constant, float& linear, float& quadratic) const;

        void SetSpotProperties(float cutoff, float exponent, glm::vec3 direction);
        void GetSpotProperties(float& cutoff, float& exponent, glm::vec3& direction) const;

    private:
        LightSource(LightType type);

        LightType m_lightType;
        glm::vec4 m_position;
        glm::vec4 m_ambientColor;
        glm::vec4 m_diffuseColor;
        glm::vec4 m_specularColor;

        float m_constantAttenuation;
        float m_linearAttenuation;
        float m_quadraticAttenuation;

        float m_spotCutoff;
        float m_spotExponent;
        glm::vec3 m_spotDirection;
    };
}