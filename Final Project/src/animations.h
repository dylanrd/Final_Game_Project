#pragma once
#include <glm/vec3.hpp>
#include <vector>

struct BezierSpline {
	std::vector<glm::vec3> control_points;
	std::vector<glm::vec3> handles_left;
	std::vector<glm::vec3> handles_right;
};

struct WorldPosition
{
    glm::vec3 position;
    glm::vec3 direction;
};

glm::quat getCarOrientation(const glm::vec3& forward, const glm::vec3& up);

glm::vec3 getBezierPoint(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t);

glm::vec3 getBezierDirection(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t, bool flip);

WorldPosition getPointOnCompositeCurve(const std::vector<BezierSpline>& splines, float tComposite, int animNumber);

std::vector<WorldPosition> getCompositeCurve(const std::vector<BezierSpline>& splines, int subdivisions);

std::vector<BezierSpline> loadSplinesFromJSON(const std::string& filePath);


