#ifndef HEIGHTGENERATOR_H
#define HEIGHTGENERATOR_H



#include <framework/disable_all_warnings.h>
#include <framework/window.h>
#include "glm/fwd.hpp"
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <cstdlib> 
#include <iostream>

class HeightGenerator {
public:

    HeightGenerator();
    float generateHeight(int x, int z);
    float generateSmoothHeight(int x, int z);
    float getHeight(int x, int z);
    float interpolate(float a, float b, float blend);
    float interpolateNoise(float a, float b);
    void changeSeed();
private:
    
private:
    int randomSeed = rand();
};





#endif HEIGHTGENERATOR_H