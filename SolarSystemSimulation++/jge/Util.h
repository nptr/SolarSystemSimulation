#include <glm/glm.hpp>

namespace jge
{
    class Util
    {
    public:
        static int RaySphereIntersection(glm::vec3& rayPos, glm::vec3& rayDir, glm::vec3& spherePos, float sphereRadius, float& d1, float& d2);
    };
}