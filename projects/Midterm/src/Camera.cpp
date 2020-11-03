#include "Camera.h"

#include <GLM/gtc/matrix_transform.hpp>

Camera::Camera() :
	_nearPlane(0.1f),
	_farPlane(1000.0f),
	_fovRadians(90.f),
	_aspectRatio(1.0f),
	_position(glm::vec3(0.0f)),
	_normal(glm::vec3(0.0f, 0.0f, 1.0f)),
	_up(glm::vec3(0.0f, 1.0f, 0.0f)), // Using Y-up coordinate system by default
	_view(glm::mat4(1.0f)),
	_projection(glm::mat4(1.0f)),
	_viewProjection(glm::mat4(1.0f)),
	_isDirty(true),
	_cameraType(CAMERA_PERSPECTIVE),
	_halfHeight(2.5f)
{
	__CalculateProjection();
}

Camera& Camera::SetPosition(const glm::vec3& position) {
	_position = position;
	__CalculateView();
	return *this;
}

Camera& Camera::SetForward(const glm::vec3& forward) {
	_normal = forward;
	__CalculateView();
	return *this;
}

Camera& Camera::LookAt(const glm::vec3& point) {
	_normal = glm::normalize(point - _position);
	__CalculateView();
	return *this;
}

Camera& Camera::SetRotation(const glm::quat& rotation)
{
	_normal = glm::vec3(glm::vec4(_up, 1) * rotation);
	__CalculateView();
	return *this;
}

Camera& Camera::SetUp(const glm::vec3& up) {
	_up = up;
	__CalculateView();
	return *this;
}

Camera& Camera::ResizeWindow(int windowWidth, int windowHeight) {
	_aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	__CalculateProjection();
	return *this;
}

Camera& Camera::SetFovRadians(float value) {
	_fovRadians = value;
	__CalculateProjection();
	return *this;
}

Camera& Camera::SetFovDegrees(float value) {
	SetFovRadians(glm::radians(value));
	return *this;
}

Camera& Camera::ChangePerspective(bool cameraType)
{
	_cameraType = cameraType;
	__CalculateProjection();
	return *this;
}

const glm::mat4& Camera::GetViewProjection() const {
	// TODO: implement
	if (_isDirty) {
		_viewProjection = _projection * _view;
		_isDirty = false;
	}
	return _viewProjection;
}

void Camera::__CalculateProjection()
{
	// TODO: implement
	if (_cameraType == CAMERA_PERSPECTIVE)
	{
		_projection = glm::perspective(_fovRadians, _aspectRatio, _nearPlane, _farPlane);
	}
	else if (_cameraType == CAMERA_ORTHO)
	{
		_projection = glm::ortho(-_halfHeight * _aspectRatio, _halfHeight * _aspectRatio,
			-_halfHeight, _halfHeight, _nearPlane, _farPlane);
	}
	_isDirty = true;
}

void Camera::__CalculateView() {
	// TODO: implement
	_view = glm::lookAt(_position, _position + _normal, _up);
	_isDirty = true;
}
