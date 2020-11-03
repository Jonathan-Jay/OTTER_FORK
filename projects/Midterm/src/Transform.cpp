#include "Transform.h"

Transform::Transform()
{

}

glm::mat4 Transform::GetModel()
{
	glm::mat4 Model = glm::translate(glm::mat4(1.f), m_position);
	Model *= glm::toMat4(m_rotation);
	Model = glm::scale(Model, m_scale);

	return Model;
}

Transform& Transform::SetPosition(const glm::vec3& pos)
{
	m_position = pos;
	return *this;
}

glm::vec3 Transform::GetPosition()
{
	return m_position;
}

Transform& Transform::SetScale(const glm::vec3& scale)
{
	m_scale = scale;
	return *this;
}

glm::vec3 Transform::GetScale()
{
	return m_scale;
}

Transform& Transform::SetRotation(const glm::mat3& rot)
{
	m_rotation = rot;
	return *this;
}

Transform& Transform::SetRotation(const glm::quat& rot)
{
	m_rotation = rot;
	return *this;
}

glm::quat Transform::GetRotation()
{
	return m_rotation;
}

glm::mat3 Transform::GetRotationM3()
{
	return glm::toMat3(m_rotation);
}
