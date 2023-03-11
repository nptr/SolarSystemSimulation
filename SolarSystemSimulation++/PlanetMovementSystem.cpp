#include "PlanetMovementSystem.h"

#include <glm/gtx/transform.hpp>
#include <cmath>

using namespace glm;

glm::mat4 PlanetMovementSystem::OrbitAroundSun(double time, const PlanetInfo& child)
{
    // calculate with double precision!
    float angle = (float)(360.0 * fmod(time, (double)child.roundTripTime) / (double)child.roundTripTime);
    float spinAngle = (float)(360.0 * fmod(time, (double)child.selfRotationTime) / (double)child.selfRotationTime);

    mat4 orbit = glm::rotate(glm::radians(angle), vec3(0, 1, 0));							// orbit around sun
    mat4 translate = glm::translate(vec3(child.distanceToParent, 0, 0));						// translate away from sun
    mat4 inclinate = glm::rotate(child.equatorInclination, vec3(0, 0, 1));						// equator inclination
    mat4 spin = glm::rotate(glm::radians(spinAngle), vec3(0, 1, 0));						// spin (self rotation)
    mat4 scale = glm::scale(vec3(child.planetSize, child.planetSize, child.planetSize));	// scale the object
    mat4 orbitInclinate = glm::rotate(child.orbitInclination, vec3(0, 0, 1));						// orbits inclination

    // Beware: Matrix multiplication order is important, read RTL!
    // First we scale the object at the origin (0,0,0)
    // Then we spin it (still at the origin) -> self rotation / spin
    // Now we translate it away and rotate it afterwards -> orbit around sun
    mat4 planetMatrix = orbitInclinate * orbit * translate * spin * scale;

    // Here we store the translation matrix to and from the origin
    mat4 toOrigin = glm::translate(-vec3(planetMatrix[3]));
    mat4 toLocation = glm::translate(vec3(planetMatrix[3]));

    // We translate the object back to the origin and apply the inclination matrix
    // As a last step we bring it back to the previous position
    planetMatrix = toLocation * inclinate * toOrigin * planetMatrix;

    return planetMatrix;
}


glm::mat4 PlanetMovementSystem::OrbitAroundParent(double time, const PlanetInfo& child, const vec3& parentPos)
{
    float childAngle = (float)(360.0 * fmod(time, (double)child.roundTripTime) / (double)child.roundTripTime);

    mat4 toParentPos = glm::translate(parentPos);
    mat4 childRotate = glm::rotate(glm::radians(childAngle), vec3(0, 1, 0));
    mat4 childTranslate = glm::translate(vec3(child.distanceToParent, 0, 0));
    mat4 childScale = glm::scale(vec3(child.planetSize, child.planetSize, child.planetSize));

    // Like in OrbitAroundSun()
    mat4 childMatrix = toParentPos * childRotate * childTranslate * childScale;

    // Here we store the translation matrix to and from the origin
    mat4 toOrigin = glm::translate(-vec3(childMatrix[3]));
    mat4 toLocation = glm::translate(vec3(childMatrix[3]));
    mat4 inclinate = glm::rotate(child.equatorInclination, vec3(0, 0, 1));

    childMatrix = toLocation * inclinate * toOrigin * childMatrix;
    return childMatrix;
}


glm::mat4 PlanetMovementSystem::SimulateRing(double time, const PlanetInfo& ring, const glm::vec3& parentPos)
{
    mat4 toParentPos = glm::translate(parentPos);
    mat4 childScale = glm::scale(vec3(ring.planetSize, 1.0f, ring.planetSize));

    // scale the ring and bring it to its parent
    mat4 ringMatrix = toParentPos * childScale;

    // Here we store the translation matrix to and from the origin
    mat4 toOrigin = glm::translate(-vec3(ringMatrix[3]));
    mat4 toLocation = glm::translate(vec3(ringMatrix[3]));
    mat4 inclinate = glm::rotate(ring.equatorInclination, vec3(0, 0, 1));

    // inclinate the ring
    ringMatrix = toLocation * inclinate * toOrigin * ringMatrix;
    return ringMatrix;
}