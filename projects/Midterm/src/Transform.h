#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform
{
public:
	Transform();

	glm::mat4 GetModel();

	Transform& SetPosition(const glm::vec3& pos);
	glm::vec3 GetPosition();

	Transform& SetScale(const glm::vec3& scale);
	glm::vec3 GetScale();

	Transform& SetRotation(const glm::mat3& rot);
	Transform& SetRotation(const glm::quat& rot);
	glm::quat GetRotation();
	glm::mat3 GetRotationM3();

private:
	glm::vec3 m_position = glm::vec3(0.f);
	glm::vec3 m_scale = glm::vec3(1.f);
	glm::quat m_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
};