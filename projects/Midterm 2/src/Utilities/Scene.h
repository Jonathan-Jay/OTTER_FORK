#pragma once
#include "Rendering.h"
#include "Input.h"
#include "Components/Collisions.h"

class Scene
{
public:
	Scene(const std::string& name);
	~Scene();

	entt::registry* GetRegistry();
	std::string GetName();

	virtual void Init(int windowWidth, int windowHeight);

	virtual Scene* Reattach();

	virtual void Update() {}
	virtual void Exit() {}
	void BackEndUpdate();

protected:
	std::string m_name;
	float m_dt = 0;
	float m_lastClock = 0;
	short m_camCount = 1;

	entt::registry m_reg;
};

