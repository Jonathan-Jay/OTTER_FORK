#pragma once
#include "Engine/BackEnd.h"
#include "Components/ECS.h"
#include "Components/ObjMorphLoader.h"

namespace Rendering
{
	void Update(entt::registry* reg, int numOfCams, float dt);

	extern glm::vec4 BackColour;
	extern glm::vec3 LightPos1;
	extern glm::vec3 LightPos2;
	extern glm::vec3 LightColour;
	extern glm::vec3 DefaultColour;

	extern bool massDraw;
};

