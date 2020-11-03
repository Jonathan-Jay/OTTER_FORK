#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>

class Bullet
{
public:
	Bullet() {}
	void Init(glm::quat rotation, float speed, int bounceCount, unsigned shooter) {
		movement = glm::rotate(rotation, glm::vec3(0, 0, speed));
		boing = bounceCount;
		player = shooter;
	}
	int FlipZ() { movement.z = -movement.z; return --boing; }
	int FlipX() { movement.x = -movement.x; return --boing; }
	int Reflect(glm::vec3 axis) { movement = glm::reflect(movement, axis); return --boing; }

	glm::vec3 getMovement() { return movement; }
	unsigned getShooter() { return player; }

private:
	glm::vec3 movement = glm::vec3(0);
	int boing = 0;
	unsigned player = 0;
};

