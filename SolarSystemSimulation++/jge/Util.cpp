#include "Util.h"

using namespace jge;

int Util::RaySphereIntersection(glm::vec3& rayPos, glm::vec3& rayDir, glm::vec3& spherePos, float sphereRadius, float& dist1, float& dist2)
{
    glm::vec3 o_minus_c = rayPos - spherePos;

    float p = glm::dot(rayDir, o_minus_c);
    float q = glm::dot(o_minus_c, o_minus_c) - (sphereRadius * sphereRadius);
    
    float discriminant = (p * p) - q;
    if (discriminant < 0.0f)
    {
        return 0;
    }

    float dRoot = sqrt(discriminant);
    dist1 = -p - dRoot;
    dist2 = -p + dRoot;

    return (discriminant > 1e-7) ? 2 : 1;
}