#pragma once

struct PlanetInfo
{
    /**
    * Initializes a plant info structure
    * @param size   Diameter in OpenGL units
    * @param rtt    Round-trip-time around parent in days
    * @param sr     Earth Days for one self-revolution
    * @param eqIncl Equator inclination in rad
    * @param orbInc Orbit inclination in rad
    */
    PlanetInfo(float size, float rtt, float dist, float sr = 0.0f, float eqIncl = 0.0f, float orbInc = 0.0f)
        : planetSize(size)
        , roundTripTime(rtt)
        , distanceToParent(dist)
        , selfRotationTime(sr)
        , equatorInclination(eqIncl)
        , orbitInclination(orbInc)
    {
    }

    float planetSize;
    float roundTripTime;
    float distanceToParent;
    float selfRotationTime;
    float equatorInclination;
    float orbitInclination;
};