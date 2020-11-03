#include "BackEndThings/Rendering.h"

//Jonathan Jay - 100743575 < soloed it

GLFWwindow* window;

glm::vec3 CamShake(float t) {
	t = 0.5f * std::cbrtf(2 * t - 1) + 0.5f;
	return glm::vec3(rand() % 61 - 30, rand() % 31 - 15, rand() % 41 - 20) * t * 0.005f;
}

int main() {
	srand(time(0));
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	std::cout << "\nBattle Tank  -  Jonathan Jay - 100743575\n"
		"Controls are as follows:\n"
		"\t\tP1 (RED):\t\tP2 (BLUE):\n"
		"\t-Move:\tWASD\t\t\tIJKL\n"
		"\t-Shoot:\tLeftShift or C\t\tN or .\n\n"
		"\tGlobal:\n"
		"\t-Tab to toggle persective\n"
		"\t-SpaceBar to toggle screen shake\n"
		"\t-ESC to reset\n"
		"Whenever the game is reset, it switches between a set map and a randomized map\n";

	if (!(window = BackEnd::Init("Battle Tank")))	return 1;

	int width = 1550;
	int height = 750;

	glfwSetWindowSize(window, width, height);
	glViewport(0, 0, width, height);
	Rendering::BackColour = glm::vec4(0, 0, 0, 0);

	ObjLoader::Init();

	//preloading meshes
	ObjLoader("models/explosion.obj", true).LoadMesh("models/Ball.obj").
		LoadMesh("models/num/9.obj").LoadMesh("models/num/8.obj").LoadMesh("models/num/7.obj").
		LoadMesh("models/num/6.obj").LoadMesh("models/num/5.obj").LoadMesh("models/num/4.obj").
		LoadMesh("models/num/3.obj").LoadMesh("models/num/2.obj").LoadMesh("models/num/1.obj").
		LoadMesh("models/num/0.obj").LoadMesh("models/Cube.obj").LoadMesh("models/Pillar.obj");

	// some data stuff
	Collisions::SetWorldBounds(glm::vec2(-20, -10), glm::vec2(20, 10), glm::vec2(0, -60));
	glm::vec3 vecUp = glm::vec3(0, 1, 0);
	glm::vec3 camPos = glm::vec3(0.f, 10.f, -4.f);
	glm::vec3 player1pos = glm::vec3(17.5f, 0.5f, 0);
	glm::vec3 player2pos = glm::vec3(-17.5f, 0.5f, 0);
	constexpr float p1rot = glm::radians(90.f);
	constexpr float p2rot = glm::radians(-90.f);
	float speed = 5;
	float bulletSpeed = 7.5f;
	float stunTime = 0.25f;
	bool randomMap = false;
	bool escPressed = false;
	bool spacePressed = false;
	bool tabPressed = false;
	bool screenshake = false;
	bool camOrtho = CAMERA_ORTHO;

	while (!glfwWindowShouldClose(window)) {

		/// Creating Entities
		entt::registry reg;
		ECS::AttachRegistry(&reg);

		unsigned cameraEnt = ECS::CreateEntity();
		ECS::AttachComponent<Camera>(cameraEnt).ChangePerspective(camOrtho).
			SetOrtho(20.5f).SetAspect(width / float(height)).SetFovDegrees(45.f);
		ECS::GetComponent<Transform>(cameraEnt).SetPosition(camPos).
			SetRotation(glm::normalize(glm::quat(0.2f, -0.95f, 0, 0)));

		//order of things here affect the default colour values, "it jsut works"
		unsigned p1Score = ECS::CreateEntity();
		ECS::AttachComponent<ObjLoader>(p1Score).LoadMesh("models/num/0.obj");
		ECS::GetComponent<Transform>(p1Score).SetPosition(glm::vec3(20.5f, 3.f, 8.f));
		unsigned Player1 = ECS::CreateEntity();
		ECS::AttachComponent<ObjLoader>(Player1).LoadMesh("models/Tank.obj");
		ECS::AttachComponent<Player>(Player1).playerData(Player1, true);
		ECS::AttachComponent<Collisions>(Player1).Init(0.5f, true, false);

		unsigned p2Score = ECS::CreateEntity();
		ECS::AttachComponent<ObjLoader>(p2Score).LoadMesh("models/num/0.obj");
		ECS::GetComponent<Transform>(p2Score).SetPosition(glm::vec3(-20.5f, 3.f, 8.f));
		unsigned Player2 = ECS::CreateEntity();
		ECS::AttachComponent<ObjLoader>(Player2).LoadMesh("models/Tank.obj");
		ECS::AttachComponent<Player>(Player2).playerData(Player2, false);
		ECS::AttachComponent<Collisions>(Player2).Init(0.5f, true, false);

		//Map is spawned after since it will be drawn first, making the player explosion transparency work
		unsigned Map = ECS::CreateEntity();
		ECS::AttachComponent<ObjLoader>(Map).LoadMesh("models/Map.obj", true);

		if (randomMap) {
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

		Transform* p1 = &ECS::GetComponent<Transform>(Player1).SetRotation(glm::angleAxis(p1rot, vecUp));
		Transform* p2 = &ECS::GetComponent<Transform>(Player2).SetRotation(glm::angleAxis(p2rot, vecUp));
		Rendering::LightPos1 = p1->SetPosition(player1pos).GetPosition();
		Rendering::LightPos2 = p2->SetPosition(player2pos).GetPosition();

		/// End of creating entities

		float p1Stun = 1;
		float p2Stun = 1;

		float respawnTimer = p1Stun + 3;
		float lastClock = glfwGetTime();

		std::cout << std::setfill(' ') << "\n--New Game--\nScreenshake: " << (screenshake ? "ON \r" : "OFF\r");
		//std::cout << std::setfill(' ') << "\n--New Game--\n\t Screenshake: " << (screenshake ? "ON \r" : "OFF\r");

		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			float deltaTime = glfwGetTime() - lastClock;
			lastClock = glfwGetTime();
			//std::cout << "FPS:" << std::fixed << std::setprecision(0) << std::setw(4) << 1.f / deltaTime << '\r';

			/// Start of loop
			auto& p1p = ECS::GetComponent<Player>(Player1);
			auto& p2p = ECS::GetComponent<Player>(Player2);
			p1 = &ECS::GetComponent<Transform>(Player1);
			p2 = &ECS::GetComponent<Transform>(Player2);

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
					if ((glfwGetKey(window, GLFW_KEY_C) || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) && respawnTimer == 0) {
						if (p1p.Shoot(bulletSpeed, p1->GetRotation(), p1->GetPosition())) {
							p1Stun = stunTime;
							p1move.z -= 0.25f / deltaTime;
						}
					}
					if (respawnTimer < 1) {
						if (glfwGetKey(window, GLFW_KEY_A))		p1move.x += speed;
						if (glfwGetKey(window, GLFW_KEY_D))		p1move.x -= speed;
						if (glfwGetKey(window, GLFW_KEY_W))		p1move.z += speed;
						if (glfwGetKey(window, GLFW_KEY_S))		p1move.z -= speed;
					}
					if (p1move.x != 0)
						p1->SetRotation(glm::rotate(p1->GetRotation(), p1move.x * deltaTime / 2.f, vecUp));
					else if (p1move.z != 0)
						p1->SetPosition(p1->GetPosition() + glm::rotate(p1->GetRotation(), p1move * deltaTime));
				}
				else {
					p1Stun -= deltaTime;
					if (p1Stun <= 0) {
						p1Stun = 0;
						if (respawnTimer != 0) {
							p1->SetPosition(player1pos).SetRotation(glm::angleAxis(p1rot, vecUp));
							ECS::GetComponent<ObjLoader>(Player1).LoadMesh("models/Tank.obj");
						}
					}
				}
			}

			// player 2 controls
			{
				if (p2Stun == 0) {
					if ((glfwGetKey(window, GLFW_KEY_N) || glfwGetKey(window, GLFW_KEY_PERIOD)) && respawnTimer == 0) {
						if (p2p.Shoot(bulletSpeed, p2->GetRotation(), p2->GetPosition())) {
							p2Stun = stunTime;
							p2move.z -= 0.25f / deltaTime;
						}
					}
					if (respawnTimer < 1) {
						if (glfwGetKey(window, GLFW_KEY_J))		p2move.x += speed;
						if (glfwGetKey(window, GLFW_KEY_L))		p2move.x -= speed;
						if (glfwGetKey(window, GLFW_KEY_I))		p2move.z += speed;
						if (glfwGetKey(window, GLFW_KEY_K))		p2move.z -= speed;
					}
					if (p2move.x != 0)
						p2->SetRotation(glm::rotate(p2->GetRotation(), p2move.x * deltaTime / 2.f, vecUp));
					else if (p2move.z != 0)
						p2->SetPosition(p2->GetPosition() + glm::rotate(p2->GetRotation(), p2move * deltaTime));
				}
				else {
					p2Stun -= deltaTime;
					if (p2Stun <= 0) {
						p2Stun = 0;
						if (respawnTimer != 0) {
							p2->SetPosition(player2pos).SetRotation(glm::angleAxis(p2rot, vecUp));
							ECS::GetComponent<ObjLoader>(Player2).LoadMesh("models/Tank.obj");
						}
					}
				}
			}

			if (respawnTimer > 0) {
				respawnTimer -= deltaTime;
				if (respawnTimer <= 0)
					respawnTimer = 0;
				if (screenshake) {
					ECS::GetComponent<Transform>(cameraEnt).SetPosition(camPos + CamShake((p1Stun + p2Stun + respawnTimer) / 4.f));
				}
			}

			//the light follows the bullet if it's been shot
			unsigned entity = 0;
			if ((entity = p1p.getBulletId()) != 0)
				Rendering::LightPos1 = ECS::GetComponent<Transform>(entity).GetPosition();
			else								Rendering::LightPos1 = p1->GetPosition();

			if ((entity = p2p.getBulletId()) != 0)
				Rendering::LightPos2 = ECS::GetComponent<Transform>(entity).GetPosition();
			else								Rendering::LightPos2 = p2->GetPosition();

			if (glfwGetKey(window, GLFW_KEY_SPACE)) {
				if (!spacePressed) {
					spacePressed = true;
					//std::cout << "\t Screenshake: " << ((screenshake = !screenshake) ? "ON \r" : "OFF\r");
					std::cout << "Screenshake: " << ((screenshake = !screenshake) ? "ON \r" : "OFF\r");
					ECS::GetComponent<Transform>(cameraEnt).SetPosition(camPos);
				}
			}
			else spacePressed = false;

			if (glfwGetKey(window, GLFW_KEY_TAB)) {
				if (!tabPressed) {
					tabPressed = true;
					if (camOrtho = !camOrtho)
						camPos = glm::vec3(0.f, 25.f, -12.f);
					else
						camPos = glm::vec3(0.f, 10.f, -4.f);
					ECS::GetComponent<Camera>(cameraEnt).ChangePerspective(camOrtho);
					ECS::GetComponent<Transform>(cameraEnt).SetPosition(camPos);
				}
			}
			else tabPressed = false;

			if (glfwGetKey(window, GLFW_KEY_ESCAPE) ||
				((p1p.getScore() == 10 || p2p.getScore() == 10) && respawnTimer == 0)) {

				if (p1p.getScore() == 10) {
					std::cout << "Player 1 wins, ";
					escPressed = false;
				}
				if (p2p.getScore() == 10) {
					std::cout << "Player 2 wins, ";
					escPressed = false;
				}
				if (!escPressed) {
					//std::cout << "Score: P1 - " << p1p.getScore() << " : p2 - " << p2p.getScore() << "   \n";
					std::cout << "Score: P1 - " << p1p.getScore() << " : p2 - " << p2p.getScore() << "\n";
					randomMap = !randomMap;
					escPressed = true;
					break;
				}
			}
			else escPressed = false;

			/// End of loop
			Collisions::Update(&reg, deltaTime);
			Rendering::Update(&reg);

			glfwSwapBuffers(window);
		}

		//break out of loop when resetting
	}
	Shader::UnBind();
	VertexArrayObject::UnBind();

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}
