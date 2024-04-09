#include <framework/disable_all_warnings.h>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
DISABLE_WARNINGS_PUSH()
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
DISABLE_WARNINGS_POP()
#include "animations.h"

using json = nlohmann::json;
using namespace std;

// Function to load Bezier splines from a JSON file
vector<BezierSpline> loadSplinesFromJSON(const string& filePath) {
    // Open the file and parse the JSON
    ifstream inFile(filePath);
    json j;
    inFile >> j;

	vector<BezierSpline> splines;

    for (const auto& splineJSON : j["splines"]) {
        BezierSpline spline;
        for (const auto& cp : splineJSON["control_points"]) {
            spline.control_points.emplace_back(glm::vec3(cp[0], cp[2], -cp[1]));
        }
        for (const auto& hl : splineJSON["handles_left"]) {
            spline.handles_left.emplace_back(glm::vec3(hl[0], hl[2], -hl[1]));
        }
        for (const auto& hr : splineJSON["handles_right"]) {
            spline.handles_right.emplace_back(glm::vec3(hr[0], hr[2], -hr[1]));
        }
        splines.emplace_back(spline);
    }

    return splines;
}



// using 2 bezier splines, calculates a point on the resulting bezier curve based on float t in [0,1]
glm::vec3 getBezierPoint(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 p = uuu * p0; // first term
    p += 3 * uu * t * p1; // second term
    p += 3 * u * tt * p2; // third term
    p += ttt * p3; // fourth term

    return p;
}


//get object direction when following a bezier curve
glm::vec3 getBezierDirection(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t, bool flip = false) {
    glm::vec3 tangent =  3.0f * (1.0f - t) * (1.0f - t) * (p1 - p0) +
        6.0f * (1.0f - t) * t * (p2 - p1) +
        3.0f * t * t * (p3 - p2);
    if (flip) {
        tangent = -tangent; // Flip the direction if flip is true
    }
    return glm::normalize(tangent); // Return the normalized direction vector
}

//calculates length of each individual bezier curve
float getBezierLength(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, int subdivisions = 20) {
    float length = 0.0f;
    glm::vec3 prevPoint = p0;

    for (int i = 1; i <= subdivisions; ++i) {
        float t = i / static_cast<float>(subdivisions);
        glm::vec3 point = getBezierPoint(p0, p1, p2, p3, t);
        length += glm::distance(prevPoint, point);
        prevPoint = point;
    }

    return length;
}



//function to get a point on the larger composite curve
WorldPosition getPointOnCompositeCurve(const std::vector<BezierSpline>& splines, float tComposite) {
    std::vector<float> segmentLengths;
    float totalLength = 0.0f;
    BezierSpline spline = splines[0];

    //iterate through all splines and calculate length of each segment
    for (size_t i = 0; i < spline.control_points.size() - 1; i += 3) {
        float length = getBezierLength(spline.control_points[i], spline.handles_right[i],
            spline.handles_left[i + 1], spline.control_points[i + 1]);
        segmentLengths.push_back(length);
        totalLength += length;
    }

    //normalize tComposite to [0, 1]
    float tNormalized = tComposite / 100.0f;
    float cumulativeProportion = 0.0f;
    int segmentIndex = 0;
    float tSegment = 0.0f;

    //get corresponding segment based on position on larger curve, as well as the t value for that segment
    for (size_t i = 0; i < segmentLengths.size(); ++i) {
        float segmentProportion = segmentLengths[i] / totalLength;
        if (tNormalized < cumulativeProportion + segmentProportion) {
            segmentIndex = i;
            tSegment = (tNormalized - cumulativeProportion) / segmentProportion;
            break;
        }
        cumulativeProportion += segmentProportion;
    }
    int pointsPerSegment = 3; // This example assumes 4 points (P0, P1/P2 handles, P3) define each segment.

    // For segmentIndex found from mapping tComposite to tNormalized
    int startIndex = segmentIndex * pointsPerSegment;

    // Adjust indices based on your data structure
    const auto& p0 = spline.control_points[startIndex];
    const auto& p1 = spline.handles_right[startIndex];
    const auto& p2 = spline.handles_left[startIndex + 1]; // Assuming next control point's left handle
    const auto& p3 = spline.control_points[startIndex + 1]; // The next control point

    WorldPosition result = { getBezierPoint(p0, p1, p2, p3, tSegment), getBezierDirection(p0, p1, p2, p3, tSegment) };
    return result;
}

//car orientation based on forward direction and up direction
glm::quat getCarOrientation(const glm::vec3& forward, const glm::vec3& up) {
    // Normalize the forward direction
    glm::vec3 resForward = glm::normalize(forward);

    // Calculate the right vector
    glm::vec3 resRight = glm::normalize(glm::cross(up, resForward));

    // Recalculate the 'up' vector to ensure orthogonality
    glm::vec3 resUp = glm::cross(resForward, resRight);

    // Construct a rotation matrix from the direction vectors
    glm::mat3 rotationMatrix;
    rotationMatrix[0] = resRight;          // Right
    rotationMatrix[1] = resUp;               // Up
    rotationMatrix[2] = resForward;        // Forward

    // Convert the rotation matrix to a quaternion
    glm::quat orientation = glm::quat(rotationMatrix);

    return orientation;
}
