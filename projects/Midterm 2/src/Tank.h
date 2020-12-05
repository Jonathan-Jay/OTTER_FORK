#pragma once
#include "Utilities/Scene.h"

class Tank : public Scene
{
public:
	Tank(const std::string& name) :
		Scene(name) {}
	~Tank() {}

	void Init(int windowWidth, int windowHeight) override;
	void Update() override;

private:
	glm::vec3 vecUp = glm::vec3(0, 1, 0);
	glm::vec3 camPos = glm::vec3(0.f, 10.f, 3.f);
	glm::vec3 player1pos = glm::vec3(-17.5f, 0.5f, 0);
	glm::vec3 player2pos = glm::vec3(17.5f, 0.5f, 0);
	const float p1rot = glm::radians(-90.f);
	const float p2rot = glm::radians(90.f);
	float speed = 5;
	float bulletSpeed = 7.5f;
	float stunTime = 0.25f;
	float p1Stun = 1;
	float p2Stun = 1;
	float respawnTimer = p1Stun + 3;

	bool randomMap = false;
	bool screenshake = true;

	unsigned cameraEnt = 0;
	unsigned Player1 = 0;
	unsigned p1Score = 0;
	unsigned Player2 = 0;
	unsigned p2Score = 0;
};

