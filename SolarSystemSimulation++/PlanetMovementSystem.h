#pragma once

#include <glm/glm.hpp>
#include "PlanetInfo.h"

class PlanetMovementSystem
{
public:
    static glm::mat4 OrbitAroundSun(double time, const PlanetInfo& child);
    static glm::mat4 OrbitAroundParent(double time, const PlanetInfo& child, const glm::vec3& parentPos);
    static glm::mat4 SimulateRing(double time, const PlanetInfo& ring, const glm::vec3& parentPos);
};

