#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>

#include "Quaternion.h"
#include "Inputs.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// Spherical Coordinate Camera.
//   phi: angle on Y plane
// theta: angle perpendicular to XZ plane
//     d: distance to object
class Camera
{
public:
    Camera(void);
    ~Camera(void);

    static Vector3 SphericalToVec3(double phi, double theta, double d);

    Vector3 GetViewVector() const;
    XMMATRIX GetViewMatrix() const;

    void Translate(Vector3 offset);
    void Update(double lastTimeStep, Inputs* inputs);

    static const double kCameraSpeed;

    Vector3 position;
    Vector3 up;
    Vector3 lookAt;

    Framework::Quaternion orientation;
    double phi = -66.8; // y
    double theta = 0.55; // xz
    double d = 1.0;
};
