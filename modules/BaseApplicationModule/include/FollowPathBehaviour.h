#pragma once
#include "IBehaviour.h"
#include <vector>
#include <GLM/glm.hpp>
#include <GLM/gtc/quaternion.hpp>

class FollowPathBehaviour final : public IBehaviour
{
public:
	struct Pointt
	{
		glm::vec3 pos = glm::vec3();
		glm::quat rot = glm::quat();
	};

	FollowPathBehaviour() :
		Points(std::vector<Pointt>()),
		Speed(1.0f),
		_nextPointIx(0) { }
	~FollowPathBehaviour() override = default;

	std::vector<Pointt> Points;
	float               Speed;

	void Update(entt::handle entity) override;
	
private:
	int _nextPointIx;
};