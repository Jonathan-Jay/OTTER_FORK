#pragma once
#include "BackEndThings/BackEnd.h"
#include "ECS.h"
#include "ObjLoader.h"
#include "Player.h"

namespace Rendering
{
	void Update(entt::registry* reg);

	extern glm::vec4 BackColour;
	extern glm::vec3 LightPos1;
	extern glm::vec3 LightPos2;
	extern glm::vec3 LightColour;
	extern glm::vec3 DefaultColour;
};

