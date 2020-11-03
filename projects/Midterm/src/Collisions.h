#pragma once
#include "dependencies/entt.hpp"
#include <glm/glm.hpp>

class Collisions
{
public:
	Collisions() {}
	Collisions(float radius, bool isDynamic, bool isBreakable);
	Collisions(float width, float height, bool isDynamic, bool isBreakable);

	void Init(float radius, bool isDynamic, bool isBreakable);
	void Init(float width, float height, bool isDynamic, bool isBreakable);

	bool isDead() { return dead; }
	void setDead(bool answer) { dead = answer; }

	static void SetWorldBounds(glm::vec2 BL, glm::vec2 TR, glm::vec2 g);

	static void Update(entt::registry* reg, float deltaTime);

private:
	static glm::vec2 BottomLeft;
	static glm::vec2 TopRight;
	static glm::vec2 Gravity;

	bool dead = false;
	glm::vec2 velo = glm::vec2(0.f);
	bool isCircle = false;
	bool isStatic = false;
	bool breakable = false;
	float width = 0;
	float height = 0;
};