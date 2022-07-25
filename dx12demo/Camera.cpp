#include "Camera.h"

const double Camera::kCameraSpeed = 16.0;

Camera::Camera(void) :
    position(-4.5, 1.0, 0.5),
    up(0.0, 1.0, 0.0),
    lookAt(0.0, 0.0, 0.0)
{
    d = (position - lookAt).Length();
    this->position = this->SphericalToVec3(this->phi, this->theta, this->d);
}

Camera::~Camera(void)
{

}

XMMATRIX Camera::GetViewMatrix() const
{
    return SimpleMath::Matrix::CreateLookAt(position, lookAt, up);
}

Vector3 Camera::GetViewVector() const
{
    Vector3 r = lookAt - position;
    r.Normalize();
    return r;
}

void Camera::Translate(Vector3 offset)
{
    this->position += offset;
}

void Camera::Update(double lastTimeStep, Inputs* inputs)
{
    if (inputs->IsDragging(Inputs::MB_MOUSE_RIGHT))
    {
        const int64_t deltaX = inputs->mouseX - inputs->mouseXLast;
        this->phi += deltaX * lastTimeStep;
        const int64_t deltaY = inputs->mouseYLast - inputs->mouseY;
        this->theta += deltaY * lastTimeStep;
    }

    if (inputs->MouseWheelChanged())
    {
        double sign = 1.0;
        if (inputs->mousewheelDelta > 0)
        {
            sign = -1.0;
        }
        this->d += sign * (10.0 * kCameraSpeed * lastTimeStep);
    }

    this->position = this->SphericalToVec3(this->phi, this->theta, this->d);
}

Vector3 Camera::SphericalToVec3(double phi, double theta, double d)
{
    const double t = d * cos(theta);
    const double y = d * sin(theta);
    const double x = t * cos(phi);
    const double z = t * sin(phi);
    return Vector3(static_cast<float>(x),
                   static_cast<float>(y),
                   static_cast<float>(z));
}
