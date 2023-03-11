#include "LightSource.h"
#include "../gl_core_3_3.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Degree to Radiand conversion
#define RAD(x) (((x)*M_PI)/180.0f)

using namespace glm;

namespace jge
{
    LightSource* LightSource::CreatePointLight(const glm::vec3& pos)
    {
        LightSource* l = new LightSource(LightType::POINT);
        l->SetPosition(pos);
        return l;
    }
    LightSource* LightSource::CreateSpotLight(const glm::vec3& pos, const glm::vec3& dir, float cutoff, float exponent)
    {
        LightSource* l = new LightSource(LightType::SPOT);
        l->SetPosition(pos);
        l->SetSpotProperties(cutoff, exponent, dir);
        return l;
    }
    LightSource* LightSource::CreateDirectionalLight(const glm::vec3& dir)
    {
        LightSource* l = new LightSource(LightType::DIRECTIONAL);
        l->m_spotDirection = dir;
        return l;
    }

    LightSource::LightSource(LightType type)
        : m_lightType(type)
        , m_position(0, 0, 0, 1.0)
    {
        SetColor(LightComponent::AMBIENT, 0.07f, 0.07f, 0.07f);
        SetColor(LightComponent::DIFFUSE, 1.0f, 1.0f, 1.0f);
        SetColor(LightComponent::SPECULAR, 1.0f, 1.0f, 1.0f);

        /// The shader chooses the light type on
        /// the values of the following properties.
        switch (m_lightType)
        {
        case LightType::DIRECTIONAL:
        {
            m_position.w = 0.0f;
        } break;
        case LightType::SPOT:
        {
            m_position.w = 1.0f;
            m_spotCutoff = 90.0f;
        } break;
        case LightType::POINT:
        {
            m_position.w = 1.0f;
            m_spotCutoff = 180.0f;
        } break;
        default:
            assert(!"Invalid LightType!");
            break;
        }
    }


    LightSource::~LightSource()
    {

    }

    void LightSource::SetPosition(const glm::vec3& pos)
    {
        m_position.x = pos.x;
        m_position.y = pos.y;
        m_position.z = pos.z;
    }

    vec4 LightSource::GetPosition() const
    {
        return m_position;
    }


    void LightSource::SetColor(LightComponent type, float r, float g, float b)
    {
        switch (type)
        {
        case LightComponent::AMBIENT:
        {
            m_ambientColor.r = r;
            m_ambientColor.g = g;
            m_ambientColor.b = b;
            m_ambientColor.a = 1.0f;
        } break;
        case LightComponent::DIFFUSE:
        {
            m_diffuseColor.r = r;
            m_diffuseColor.g = g;
            m_diffuseColor.b = b;
            m_diffuseColor.a = 1.0f;
        } break;
        case LightComponent::SPECULAR:
        {
            m_specularColor.r = r;
            m_specularColor.g = g;
            m_specularColor.b = b;
            m_specularColor.a = 1.0f;
        } break;
        default:
            assert(!"Invalid LightComponent!");
            break;
        }
    }

    vec4 LightSource::GetColor(LightComponent type) const
    {
        switch (type)
        {
        case LightComponent::AMBIENT: return m_ambientColor;
        case LightComponent::DIFFUSE: return m_diffuseColor;
        case LightComponent::SPECULAR: return m_specularColor;
        default:
        {
            assert(!"Invalid LightComponent!");
            return vec4(0.0f, 1.0f, 1.0f, 1.0f);
        }
        }
    }

    void LightSource::SetAttenuation(float constant, float linear, float quadratic)
    {
        m_constantAttenuation = constant;
        m_linearAttenuation = linear;
        m_quadraticAttenuation = quadratic;
    }

    void LightSource::GetAttenuation(float& constant, float& linear, float& quadratic) const
    {
        constant = m_constantAttenuation;
        linear = m_linearAttenuation;
        quadratic = m_quadraticAttenuation;
    }

    void LightSource::SetSpotProperties(float cutoff, float exponent, vec3 direction)
    {
        assert(m_lightType == LightType::SPOT);

        m_spotCutoff = cutoff;
        m_spotExponent = exponent;
        m_spotDirection = direction;
    }

    void LightSource::GetSpotProperties(float& cosCutoff, float& exponent, vec3& direction) const
    {
        cosCutoff = cos(RAD(m_spotCutoff)); // precalc cos() for better performance
        exponent = m_spotExponent;
        direction = m_spotDirection;
    }
}