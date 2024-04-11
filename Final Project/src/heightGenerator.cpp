#include "heightGenerator.h"
#include <cmath>

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
DISABLE_WARNINGS_POP()


HeightGenerator::HeightGenerator(void)

{
	randomSeed = rand();
}

void HeightGenerator::changeSeed() {
	randomSeed = rand();
}

float HeightGenerator::generateHeight(int x, int z) {
	return interpolateNoise(x/8, z/8) * 40.f + interpolateNoise(x / 2, z / 2) * 10.f;
}

float HeightGenerator::generateSmoothHeight(int x, int z) {
	float sum = (getHeight(x - 1, z - 1) + getHeight(x - 1, z + 1) + getHeight(x + 1, z + 1) + getHeight(x + 1, z - 1)) / 16.f;
	sum += (getHeight(x, z - 1) + getHeight(x, z + 1) + getHeight(x + 1, z) + getHeight(x - 1, z)) / 8.f;
	sum += getHeight(x, z)/4.f;
	return sum;
}

float HeightGenerator::getHeight(int x, int z) {
	srand(x*2347 + z*45432 + randomSeed);
	/*std::cout << randomSeed << std::endl;*/
	float fl = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return fl * 2.f - 1.f;
}

float HeightGenerator::interpolate(float a, float b, float blend) {
	double theta = 3.14159265358979323846 * blend;
	float f = (1.f - cos(theta)) * 0.5f;
	return a * (1.f - f) + b * (f);
}

float HeightGenerator::interpolateNoise(float a, float b) {
	int intA = (int)a;
	int intB = (int)b;

	float fracA = intA - a;
	float fracB = intB - b;

	float v1 = generateSmoothHeight(intA, intB);
	float v2 = generateSmoothHeight(intA + 1, intB);
	float v3 = generateSmoothHeight(intA, intB + 1);
	float v4 = generateSmoothHeight(intA + 1, intB + 1);

	float i1 = interpolate(v1, v2, fracA);
	float i2 = interpolate(v3, v4, fracA);
	return interpolate(i1, i2, fracB);
}