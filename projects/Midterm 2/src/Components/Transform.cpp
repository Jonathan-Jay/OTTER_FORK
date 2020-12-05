#include "Transform.h"
#include "ECS.h"

glm::mat4 one = glm::mat4(1.f);

Transform::Transform()
{
	m_dirty = true;
}

Transform::Transform(const glm::vec3& pos)
{
	m_position = pos;
	m_dirty = true;
}

Transform::Transform(const glm::vec3& pos, const glm::quat& rot)
{
	m_position = pos;
	m_rotation = rot;
	m_dirty = true;
}

Transform::Transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
{
	m_position = pos;
	m_rotation = rot;
	m_scale = scale;
	m_dirty = true;
}

Transform& Transform::ChildTo(unsigned index)
{
	//check if it exists
	if (ECS::GetRegistry()->valid(index)) {
		m_hasParent = true;
		m_parent = index;
	}

	return *this;
}

Transform& Transform::UnChild()
{
	m_hasParent = false;
	m_parent = 0;
	m_dirty = true;

	return *this;
}

Transform& Transform::SetUsingParentScale(bool yes)
{
	m_usingParentScale = yes;
	m_dirty = true;

	return *this;
}

Transform& Transform::ComputeGlobal()
{
	//parent stuff
	if (m_hasParent) {
		//check if the parent exists, just in case
		if (ECS::GetRegistry()->valid(m_parent)) {
			if (m_usingParentScale)
				m_global = ECS::GetComponent<Transform>(m_parent).GetModel();
			else
				m_global = ECS::GetComponent<Transform>(m_parent).GetScalessModel();
		}
		else	UnChild();

	}
	else	m_global = one;

	m_global = glm::translate(m_global, m_position);
	m_global *= glm::toMat4(m_rotation);
	m_global = glm::scale(m_global, m_scale);

	m_dirty = false;

	return *this;
}

glm::mat4 Transform::GetModel()
{
	if (m_hasParent)	m_dirty = true;
	if (m_dirty)		ComputeGlobal();

	return m_global;
}

glm::mat4 Transform::GetScalessModel()
{
	glm::mat4 temp = one;
	//parent stuff
	if (m_hasParent) {
		//check if the parent exists, just in case
		if (ECS::GetRegistry()->valid(m_parent)) {
			temp = ECS::GetComponent<Transform>(m_parent).GetScalessModel();
		}
		else	UnChild();

	}
	
	return glm::translate(temp, m_position) * glm::toMat4(m_rotation);
}

Transform& Transform::SetPosition(const glm::vec3& pos)
{
	m_position = pos;
	m_dirty = true;

	return *this;
}

glm::vec3 Transform::GetLocalPosition()
{
	return m_position;
}

glm::vec3 Transform::GetGlobalPosition()
{
	return glm::vec3(m_global[3]);
}

Transform& Transform::SetScale(const glm::vec3& scale)
{
	m_scale = scale;
	m_dirty = true;

	return *this;
}

Transform& Transform::SetScale(float scale)
{
	m_scale.x = scale;
	m_scale.y = scale;
	m_scale.z = scale;
	m_dirty = true;

	return *this;
}

glm::vec3 Transform::GetScale()
{
	return m_scale;
}

Transform& Transform::SetRotation(const glm::mat3& rot)
{
	m_rotation = rot;
	m_dirty = true;

	return *this;
}

Transform& Transform::SetRotation(const glm::quat& rot)
{
	m_rotation = rot;
	m_dirty = true;

	return *this;
}

glm::quat Transform::GetLocalRotation()
{
	return m_rotation;
}

glm::quat Transform::GetGlobalRotation()
{
	return glm::mat3(m_global);
}

glm::mat3 Transform::GetLocalRotationM3()
{
	return glm::toMat3(m_rotation);
}

glm::mat3 Transform::GetGlobalRotationM3()
{
	return glm::mat3(m_global);
}

glm::vec3 Transform::GetForwards()
{
	return glm::rotate(GetGlobalRotation(), glm::vec3(0, 0, 1));
}
