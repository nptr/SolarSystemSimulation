#pragma once

#include "PlanetInfo.h"
#include <glm/glm.hpp>

#define SF 1.0

enum Bodies
{
    bBegin = 0,
    bSun = 0,
    bMercury = 1,
    bVenus = 2,
    bEarth = 3,
    bMars = 4,
    bJupiter = 5,
    bSaturn = 6,
    bUranus = 7,
    bNeptune = 8,
    bMoon = 9,
    bIo = 10,
    bEuropa = 11,
    bEnd = 12
};

const char* const bodyNames[] = {
    "Sun",
    "Mercury",
    "Venus",
    "Earth",
    "Mars",
    "Jupiter",
    "Saturn",
    "Uranus",
    "Neptune",
    "Moon",
    "Io",
    "Europa",
    "NA"
};


PlanetInfo sunInfo(
    6.0f * SF,
    1.0f,
    0.0f * SF,
    25.38f,
    glm::radians(7.25f));

PlanetInfo mercuryInfo(
    2.0f * SF,
    88.0f,
    14.0f * SF,
    58.646225f,
    glm::radians(0.0f),
    glm::radians(7.00487f));

PlanetInfo venusInfo(
    1.4f * SF,
    225.0f,
    20.0f * SF,
    243.0187f,
    glm::radians(177.3f),
    glm::radians(3.39471f));

PlanetInfo earthInfo(
    1.5f * SF,
    365.0f,
    28.0f * SF,
    1.0f,
    glm::radians(23.45f));

PlanetInfo marsInfo(
    1.20f * SF,
    687.0f,
    38.0f * SF,
    1.02595675f,
    glm::radians(25.19f),
    glm::radians(1.85061f));

PlanetInfo jupiterInfo(
    4.0f * SF,
    4329.0f,
    56.0f * SF,
    0.41354f,
    glm::radians(3.12f),
    glm::radians(1.30530f));

PlanetInfo saturnInfo(
    3.5f * SF,
    10751.0f,
    73.0f * SF,
    0.44401f,
    glm::radians(26.73f),
    glm::radians(2.48446f));

PlanetInfo uranusInfo(
    2.2f * SF,
    30664.0f,
    96.0f * SF,
    0.71833f,
    glm::radians(97.86f),
    glm::radians(0.76986f));

PlanetInfo neptuneInfo(
    1.5f * SF,
    60148.0f,
    112.0f * SF,
    0.67125f,
    glm::radians(29.58f),
    glm::radians(1.76917f));

// Moon data
PlanetInfo ioInfo(0.36f * SF, 14.0f, 4.8f * SF);
PlanetInfo europaInfo(0.31f * SF, 28.0f, 5.8f * SF);
PlanetInfo moonInfo(0.3f * SF, 27.0f, 2.0f * SF);

// Misusing the planet info structure to hold the ring data
// size and inclincation angle
PlanetInfo saturnRing(3.2f, 0.0f, 0.0f, 0.0f, glm::radians(26.73f));

PlanetInfo planetInfo[] = {
    sunInfo,
    mercuryInfo,
    venusInfo,
    earthInfo,
    marsInfo,
    jupiterInfo,
    saturnInfo,
    uranusInfo,
    neptuneInfo,
    moonInfo,
    ioInfo,
    europaInfo
};