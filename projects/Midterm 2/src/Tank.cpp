#include "Tank.h"
#include "Components/Player.h"

void Tank::Init(int windowWidth, int windowHeight)
{
	Collisions::SetWorldBounds(glm::vec2(-20, -10), glm::vec2(20, 10), glm::vec2(0, 60));

	ECS::AttachRegistry(&m_reg);

	cameraEnt = ECS::CreateEntity();
	ECS::AttachComponent<Camera>(cameraEnt).SetIsOrtho(!ECS::GetComponent<Camera>(cameraEnt).GetIsOrtho()).
		SetOrthoHeight(10.25f).ResizeWindow(windowWidth, windowHeight).SetFovDegrees(45.f);
	ECS::GetComponent<Transform>(cameraEnt).SetPosition(camPos).SetRotation(glm::angleAxis(glm::radians(70.f), glm::vec3(-1, 0, 0)));

	//order of things here affect the default colour values, "it just works"
	p1Score = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(p1Score).LoadMesh("models/num/0.obj");
	ECS::GetComponent<Transform>(p1Score).SetPosition(glm::vec3(-20.5f, 3.f, -8.f));
	Player1 = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(Player1).LoadMesh("models/Tank.obj");
	ECS::AttachComponent<Player>(Player1).playerData(Player1, true);
	ECS::AttachComponent<Collisions>(Player1).Init(0.5f, true, false);

	p2Score = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(p2Score).LoadMesh("models/num/0.obj");
	ECS::GetComponent<Transform>(p2Score).SetPosition(glm::vec3(20.5f, 3.f, -8.f));
	Player2 = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(Player2).LoadMesh("models/Tank.obj");
	ECS::AttachComponent<Player>(Player2).playerData(Player2, false);
	ECS::AttachComponent<Collisions>(Player2).Init(0.5f, true, false);

	//Map is spawned after since it will be drawn first, making the player explosion transparency work
	unsigned Map = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(Map).LoadMesh("models/Map.obj", true);


	if (Input::GetKey(KEY::ENTER)) {
		{
			unsigned entity = ECS::CreateEntity();
			ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Cube.obj", true);
			ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, false);
			ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(0, 0.5f, 0));
		}
		for (int i(0); i < 500; ++i) {
			unsigned entity = ECS::CreateEntity();
			ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/PillarB.obj", true);
			ECS::AttachComponent<Collisions>(entity).Init(0.5f, true, true);
			ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(0, 0.5f, 0));
		}
		std::cout << "\nSecret map found, Hope it doesn't lag your computer :)\n";
	}
	else if (randomMap) {
		for (int i(0); i < 40; ++i)
		{
			unsigned entity = ECS::CreateEntity();
			if (rand() % 3) {
				if (rand() % 5) {
					ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Cube.obj", true);
					ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, false);
				}
				else {
					ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Pillar.obj", true);
					ECS::AttachComponent<Collisions>(entity).Init(0.5f, false, false);
				}
			}
			else {
				if (rand() % 3) {
					ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/CubeB.obj", true);
					ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, true);
				}
				else {
					ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/PillarB.obj", true);
					ECS::AttachComponent<Collisions>(entity).Init(0.5f, true, true);
				}
			}
			//ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(0, 0.5f, 9.5f - i * 2.1f));
			ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(rand() % 21 - 10, 0.5f, rand() % 18 - 8.5f));
		}
		for (int i(0); i < 7; ++i)
		{
			unsigned entity = ECS::CreateEntity();
			ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Cube.obj", true);
			ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, false);
			ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(15, 0.5f, 3.f - i));
		}
		for (int i(0); i < 7; ++i)
		{
			unsigned entity = ECS::CreateEntity();
			ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Cube.obj", true);
			ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, false);
			ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(-15, 0.5f, 3.f - i));
		}
	}
	else {
		glm::vec3 multiplier = glm::vec3(1.f);
		for (int x(0); x < 4; ++x) {
			multiplier.x = (x % 2 ? -1 : 1);
			multiplier.z = (x < 2 ? -1 : 1);
			for (int i(0); i < 3; ++i) {
				unsigned entity = ECS::CreateEntity();
				ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Cube.obj", true);
				ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, false);
				ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(15.5f - i, 0.5f, 7.5f) * multiplier);
			}
			for (int i(0); i < 3; ++i) {
				unsigned entity = ECS::CreateEntity();
				ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/CubeB.obj", true);
				ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, true);
				ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(6.5f - i, 0.5f, 5.5f) * multiplier);
			}
			{
				unsigned entity = ECS::CreateEntity();
				ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/CubeB.obj", true);
				ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, true);
				ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(6.5f, 0.5f, 4.5f) * multiplier);
			}
			for (int i(0); i < 4; ++i) {
				unsigned entity = ECS::CreateEntity();
				ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/CubeB.obj", true);
				ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, true);
				ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(15.5f, 0.5f, 3.5f - i) * multiplier);
			}
			{
				unsigned entity = ECS::CreateEntity();
				ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/CubeB.obj", true);
				ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, true);
				ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(16.5f, 0.5f, 3.5f) * multiplier);
			}
			for (int i(0); i < 2; ++i) {
				unsigned entity = ECS::CreateEntity();
				ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Cube.obj", true);
				ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, false);
				ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(0.5f, 0.5f, 9.5f - i) * multiplier);
			}
			for (int i(0); i < 2; ++i) {
				unsigned entity = ECS::CreateEntity();
				ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Cube.obj", true);
				ECS::AttachComponent<Collisions>(entity).Init(1, 1, false, false);
				ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(10.5f - i, 0.5f, 0.5f) * multiplier);
			}
			{
				unsigned entity = ECS::CreateEntity();
				ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Pillar.obj", true);
				ECS::AttachComponent<Collisions>(entity).Init(0.5f, false, false);
				ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(2.5f, 0.5f, 2.5f) * multiplier);
			}
			{
				unsigned entity = ECS::CreateEntity();
				ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/PillarB.obj", true);
				ECS::AttachComponent<Collisions>(entity).Init(0.5f, false, true);
				ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(1.5f, 0.5f, 1.5f) * multiplier);
			}
		}
	}

	Rendering::LightPos1 = ECS::GetComponent<Transform>(Player1).SetRotation(glm::angleAxis(p1rot, vecUp)).
		SetPosition(player1pos).GetGlobalPosition();
	Rendering::LightPos2 = ECS::GetComponent<Transform>(Player2).SetRotation(glm::angleAxis(p2rot, vecUp)).
		SetPosition(player2pos).GetGlobalPosition();

	/// End of creating entities
	p1Stun = 1;
	p2Stun = 1;
	respawnTimer = p1Stun + 3;

	std::cout << "Screenshake: " << (screenshake ? "ON \r" : "OFF\r");
	//std::cout << std::setfill(' ') << "\n--New Game--\n\t Screenshake: " << (screenshake ? "ON \r" : "OFF\r");
}

glm::vec3 CamShake(float t) {
	t = 0.5f * std::cbrtf(2 * t - 1) + 0.5f;
	return glm::vec3(rand() % 61 - 30, rand() % 31 - 15, rand() % 41 - 20) * t * 0.005f;
}

void Tank::Update()
{
	auto& p1 = ECS::GetComponent<Transform>(Player1);
	auto& p2 = ECS::GetComponent<Transform>(Player2);
	auto& p1p = ECS::GetComponent<Player>(Player1);
	auto& p2p = ECS::GetComponent<Player>(Player2);

	if (p1p.respawn(Player1)) {
		unsigned temp = p1p.getBulletId();	if (temp != 0)	ECS::GetComponent<Collisions>(temp).setDead(true);
		temp = p2p.getBulletId();			if (temp != 0)	ECS::GetComponent<Collisions>(temp).setDead(true);
		ECS::GetComponent<ObjLoader>(Player1).LoadMesh("models/explosion.obj", true);
		p2p.giveScore();	p1p.bulletDead();	p2p.bulletDead();
		p1Stun = 1;			p2Stun = 0;			respawnTimer = 3;
		if ((temp = p2p.getScore()) < 10)
			ECS::GetComponent<ObjLoader>(p2Score).LoadMesh("models/num/" + std::to_string(temp) + ".obj");
		else
			ECS::GetComponent<ObjLoader>(p2Score).LoadMesh("models/num/0.obj");
	}
	if (p2p.respawn(Player2)) {
		unsigned temp = p1p.getBulletId();	if (temp != 0)	ECS::GetComponent<Collisions>(temp).setDead(true);
		temp = p2p.getBulletId();			if (temp != 0)	ECS::GetComponent<Collisions>(temp).setDead(true);
		ECS::GetComponent<ObjLoader>(Player2).LoadMesh("models/explosion.obj", true);
		p1p.giveScore();	p1p.bulletDead();	p2p.bulletDead();
		p1Stun = 0;		p2Stun = 1;			respawnTimer = 3;
		if ((temp = p1p.getScore()) < 10)
			ECS::GetComponent<ObjLoader>(p1Score).LoadMesh("models/num/" + std::to_string(temp) + ".obj");
		else
			ECS::GetComponent<ObjLoader>(p1Score).LoadMesh("models/num/0.obj");
	}

	glm::vec3 p1move = glm::vec3(0.f);
	glm::vec3 p2move = glm::vec3(0.f);

	// player 1 controls
	{
		if (p1Stun == 0) {
			if ((Input::GetKey(KEY::C) || Input::GetKey(KEY::LSHIFT)) && respawnTimer == 0) {
				if (p1p.Shoot(bulletSpeed, p1.GetLocalRotation(), p1.GetLocalPosition())) {
					p1Stun = stunTime;
					p1move.z -= 0.25f / m_dt;
				}
			}
			if (respawnTimer < 1) {
				if (Input::GetKey(KEY::A))		p1move.x += speed;
				if (Input::GetKey(KEY::D))		p1move.x -= speed;
				if (Input::GetKey(KEY::W))		p1move.z += speed;
				if (Input::GetKey(KEY::S))		p1move.z -= speed;
			}
			if (p1move.x != 0)
				p1.SetRotation(glm::rotate(p1.GetLocalRotation(), p1move.x * m_dt / 2.f, vecUp));
			else if (p1move.z != 0)
				p1.SetPosition(p1.GetLocalPosition() + glm::rotate(p1.GetLocalRotation(), p1move * m_dt));
		}
		else {
			p1Stun -= m_dt;
			if (p1Stun <= 0) {
				p1Stun = 0;
				if (respawnTimer != 0) {
					p1.SetPosition(player1pos).SetRotation(glm::angleAxis(p1rot, vecUp));
					ECS::GetComponent<ObjLoader>(Player1).LoadMesh("models/Tank.obj");
				}
			}
		}
	}

	// player 2 controls
	{
		if (p2Stun == 0) {
			if ((Input::GetKey(KEY::N) || Input::GetKey(KEY::PERIOD)) && respawnTimer == 0) {
				if (p2p.Shoot(bulletSpeed, p2.GetLocalRotation(), p2.GetLocalPosition())) {
					p2Stun = stunTime;
					p2move.z -= 0.25f / m_dt;
				}
			}
			if (respawnTimer < 1) {
				if (Input::GetKey(KEY::J))		p2move.x += speed;
				if (Input::GetKey(KEY::L))		p2move.x -= speed;
				if (Input::GetKey(KEY::I))		p2move.z += speed;
				if (Input::GetKey(KEY::K))		p2move.z -= speed;
			}
			if (p2move.x != 0)
				p2.SetRotation(glm::rotate(p2.GetLocalRotation(), p2move.x * m_dt / 2.f, vecUp));
			else if (p2move.z != 0)
				p2.SetPosition(p2.GetLocalPosition() + glm::rotate(p2.GetLocalRotation(), p2move * m_dt));
		}
		else {
			p2Stun -= m_dt;
			if (p2Stun <= 0) {
				p2Stun = 0;
				if (respawnTimer != 0) {
					p2.SetPosition(player2pos).SetRotation(glm::angleAxis(p2rot, vecUp));
					ECS::GetComponent<ObjLoader>(Player2).LoadMesh("models/Tank.obj");
				}
			}
		}
	}

	if (respawnTimer > 0) {
		respawnTimer -= m_dt;
		if (respawnTimer <= 0)
			respawnTimer = 0;
		if (screenshake) {
			ECS::GetComponent<Transform>(cameraEnt).SetPosition(camPos + CamShake((p1Stun + p2Stun + respawnTimer) / 4.f));
		}
	}

	//the light follows the bullet if it's been shot
	unsigned entity = 0;
	if ((entity = p1p.getBulletId()) != 0)
		Rendering::LightPos1 = ECS::GetComponent<Transform>(entity).GetLocalPosition();
	else								Rendering::LightPos1 = p1.GetLocalPosition();

	if ((entity = p2p.getBulletId()) != 0)
		Rendering::LightPos2 = ECS::GetComponent<Transform>(entity).GetLocalPosition();
	else								Rendering::LightPos2 = p2.GetLocalPosition();

	if (Input::GetKeyDown(KEY::SPACE)) {
		//std::cout << "\t Screenshake: " << ((screenshake = !screenshake) ? "ON \r" : "OFF\r");
		std::cout << "Screenshake: " << ((screenshake = !screenshake) ? "ON \r" : "OFF\r");
		ECS::GetComponent<Transform>(cameraEnt).SetPosition(camPos);
	}

	if (Input::GetKeyDown(KEY::TAB)) {
		if (ECS::GetComponent<Camera>(cameraEnt).ToggleOrtho().GetIsOrtho())
			camPos = glm::vec3(0.f, 10.f, 3.f);
		else
			camPos = glm::vec3(0.f, 26.f, 10.f);
		ECS::GetComponent<Transform>(cameraEnt).SetPosition(camPos);
	}

	bool exit = Input::GetKeyDown(KEY::ESC);
	if (exit || ((p1p.getScore() == 10 || p2p.getScore() == 10) && respawnTimer == 0)) {

		if (p1p.getScore() == 10) {
			std::cout << "Player 1 wins, ";
			exit = true;
		}
		if (p2p.getScore() == 10) {
			std::cout << "Player 2 wins, ";
			exit = true;
		}
		if (exit) {
			//std::cout << "Score: P1 - " << p1p.getScore() << " : p2 - " << p2p.getScore() << "   \n";
			std::cout << "Score: P1 - " << p1p.getScore() << " : p2 - " << p2p.getScore() << "\n";
			randomMap = !randomMap;
			m_reg = entt::registry();
			Init(BackEnd::GetHalfWidth() * 2, BackEnd::GetHalfHeight() * 2);
		}
	}
}
