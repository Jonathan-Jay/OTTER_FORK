#pragma once
#include "Utilities/Scene.h"

class Snake : public Scene
{
public:
	Snake(const std::string& name) :
		Scene(name) {}
	~Snake() {}

	void Init(int windowWidth, int windowHeight) override;
	void Update() override;

private:
	unsigned cameraEnt = 0;
	unsigned player = 0;
	unsigned tail = 0;
	unsigned apple = 0;
	unsigned scoreEnt1 = 0;
	unsigned scoreEnt2 = 0;
	unsigned scoreEnt3 = 0;

	unsigned score = 0;
	unsigned startingSize = 5;
	bool betterApple = false;
	bool hasTail = true;
	bool updateHasTail = false;
	bool dead = false;
	bool updateScore = true;
	bool ortho = true;
	bool paused = false;
	glm::vec2 velo = glm::vec2(0);
	glm::vec2 pos = glm::vec2(0);

	glm::vec2 lastPos = glm::vec2(0, 0);
	glm::vec2 targetPos = glm::vec2(0, 0);
	glm::vec2 lastTPos = glm::vec2(0, 0);
	glm::vec2 targetTPos = glm::vec2(0, 0);

	glm::vec2 BL = glm::vec2(-10, -10);
	glm::vec2 TR = glm::vec2(10, 10);
	float step = 0;
	float counter = 0;
	float delay = 0.25f;
	float delayInv = 1.f / delay;

};

