#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <memory>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtx/quaternion.hpp>

/// <summary>
/// Represents a simple perspective camera for use by first person or third person games
/// </summary>

#define CAMERA_ORTHO 0
#define CAMERA_PERSPECTIVE 1

class Camera
{
public:
	typedef std::shared_ptr<Camera> sptr;
	inline static sptr Create() {
		return std::make_shared<Camera>();
	}

public:
	Camera();
	virtual ~Camera() = default;

	/// <summary>
	/// Sets this camera's position in world space
	/// </summary>
	Camera& SetPosition(const glm::vec3& position);
	/// <summary>
	/// Sets the camera's forward vector, ie what way it's facing
	/// </summary>
	Camera& SetForward(const glm::vec3& forward);
	/// <summary>
	/// Sets the camera's up vector, this can be thought of as a vector sticking out of the top of the camera's head
	/// </summary>
	Camera& SetUp(const glm::vec3& up);
	/// <summary>
	/// Makes this camera look at the given point in world coordinates
	/// </summary>
	Camera& LookAt(const glm::vec3& point);

	Camera& SetRotation(const glm::quat& rotation);


	/// <summary>
	/// Notifies this camera that the window has resized, and updates our projection matrix
	/// </summary>
	/// <param name="windowWidth">The new width of the window</param>
	/// <param name="windowHeight">The new height of the window</param>
	Camera& ResizeWindow(int windowWidth, int windowHeight);
	/// <summary>
	/// Sets the vertical field of view in radians for this camera
	/// </summary>
	Camera& SetFovRadians(float value);
	/// <summary>
	/// Sets the vertical field of view in degrees for this camera
	/// </summary>
	Camera& SetFovDegrees(float value);

	///set the ortho height
	Camera& SetOrtho(float size) { _halfHeight = size / 2.f; __CalculateProjection(); return *this; }

	///set the AspectRatio
	Camera& SetAspect(float value) { _aspectRatio = value; __CalculateProjection(); return *this; }

	///switches the camera between Orthographic and perspective
	///use CAMERA_ORTHO (0) and CAMERA_PERSPECTIVE (1)
	Camera& ChangePerspective(bool cameraType);
	bool GetPerspective() { return _cameraType; }

	/// <summary>
	/// Gets the camera's position in world space
	/// </summary>
	const glm::vec3& GetPosition() const { return _position; }
	/// <summary>
	/// Gets the direction that this camera is facing in world space
	/// </summary>
	const glm::vec3& GetForward() const { return _normal; }
	/// <summary>
	/// Gets the up direction for this camera (basically a vector pointing out the top of the camera's head)
	/// </summary>
	const glm::vec3& GetUp() const { return _up; }

	/// <summary>
	/// Gets the view matrix for this camera
	/// </summary>
	const glm::mat4& GetView() const { return _view; }
	/// <summary>
	/// Gets the projection matrix for this camera
	/// </summary>
	const glm::mat4& GetProjection() const { return _projection; }
	/// <summary>
	/// Gets the combined view-projection matrix for this camera, calculating if needed
	/// </summary>
	const glm::mat4& GetViewProjection() const;

protected:
	float _nearPlane;
	float _farPlane;
	float _fovRadians;
	float _aspectRatio;

	float _halfHeight;

	glm::vec3 _position;
	glm::vec3 _normal;
	glm::vec3 _up;

	glm::mat4 _view;
	glm::mat4 _projection;

	bool _cameraType;

	// The view projection, it is mutable so we can re-calculate it during const methods
	mutable glm::mat4 _viewProjection;
	// A dirty flag that indicates whether we need to re-calculate our view projection matrix
	mutable bool      _isDirty;

	// Recalculates the projection matrix
	void __CalculateProjection();
	// Recalculates the view matrix
	void __CalculateView();
};