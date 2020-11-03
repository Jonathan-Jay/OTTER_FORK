#pragma once
#include "Collisions.h"

class Player
{
public:
	Player() {}
	void playerData(unsigned id, bool p1) { playerId = id; player1 = p1; }
	bool Shoot(float bulletSpeed, glm::quat rotation, glm::vec3 pos);
	bool respawn(unsigned id) { if (playerId != 0) return false; playerId = id; return true; }

	void bulletDead() { bulletId = 0; }
	void die() { playerId = 0; }
	void giveScore() { score++; }
	unsigned getScore() { return score; }
	unsigned getBulletId() { return bulletId; }
	bool isPlayer1() { return player1; }

private:
	unsigned bulletId = 0;
	unsigned score = 0;
	unsigned playerId = 0;
	bool player1 = true;
};

