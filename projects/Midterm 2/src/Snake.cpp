#include "Snake.h"
#include "Components/Body.h"

void Snake::Init(int windowWidth, int windowHeight)
{
	ECS::AttachRegistry(&m_reg);

	cameraEnt = ECS::CreateEntity();
	ECS::AttachComponent<Camera>(cameraEnt).SetIsOrtho(ortho).
		SetOrthoHeight(10.25f).ResizeWindow(windowWidth, windowHeight).SetFovDegrees(45.f);
	ECS::GetComponent<Transform>(cameraEnt).SetPosition(ortho ? glm::vec3(-6.75f, -1.75f, 10.f) : glm::vec3(-6.75f, -5.f, 26.f)).
		SetRotation(glm::angleAxis(glm::radians(10.f), glm::vec3(1, 0, 0)));

	player = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(player).LoadMesh("models/Cube.obj");
	ECS::GetComponent<Transform>(player).SetPosition(glm::vec3(0, 0, 0.5f)).SetScale(1.0005f);
	ECS::AttachComponent<Body>(player).InitSegments(startingSize - 1);
	ECS::GetComponent<Body>(player).HasTail(hasTail);

	tail = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(tail).LoadMesh("models/Cube.obj");
	ECS::GetComponent<Transform>(tail).SetPosition(glm::vec3(0, 0, 0.5f));

	{
		unsigned entity = ECS::CreateEntity();
		ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/SnakeMap.obj", true);
	}
	{
		unsigned entity = ECS::CreateEntity();
		ECS::AttachComponent<ObjMorphLoader>(entity).LoadMeshs("Apple", true);
		ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(-15.f, 3, 0)).SetScale(3.f);
	}
	{
		unsigned entity = ECS::CreateEntity();
		ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Apple.obj", true);
		ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(-21.f, 3, 0)).SetScale(2.25f);
	}
	{
		unsigned entity = ECS::CreateEntity();
		ECS::AttachComponent<ObjMorphLoader>(entity).LoadMeshs("Apple", true);
		ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(-21.f, -4, 0)).SetScale(3.f);
	}
	{
		unsigned entity = ECS::CreateEntity();
		ECS::AttachComponent<ObjLoader>(entity).LoadMesh("models/Apple.obj", true);
		ECS::GetComponent<Transform>(entity).SetPosition(glm::vec3(-15.f, -4, 0)).SetScale(2.25f);
	}

	apple = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(apple).LoadMesh("models/Apple.obj", true).Enable();
	ECS::AttachComponent<ObjMorphLoader>(apple).LoadMeshs("Apple", true).Disable();
	ECS::GetComponent<Transform>(apple).SetScale(0.75f);
	glm::vec3 apos = glm::vec3(rand() % 19 - 9, rand() % 19 - 9, 0.5f);
	while (apos.x == 0 && apos.y == 0) {
		apos = glm::vec3(rand() % 19 - 9, rand() % 19 - 9, 0.5f);
	}
	ECS::GetComponent<Transform>(apple).SetPosition(apos).
		SetRotation(glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)));

	scoreEnt1 = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(scoreEnt1).LoadMesh("models/num/0.obj");
	ECS::GetComponent<Transform>(scoreEnt1).SetPosition(glm::vec3(-20.f, 8.5f, 2)).SetScale(2.f).
		SetRotation(glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)));

	scoreEnt2 = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(scoreEnt2).LoadMesh("models/num/0.obj");
	ECS::GetComponent<Transform>(scoreEnt2).SetPosition(glm::vec3(-18.f, 8.5f, 2)).SetScale(2.f).
		SetRotation(glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)));
	
	scoreEnt3 = ECS::CreateEntity();
	ECS::AttachComponent<ObjLoader>(scoreEnt3).LoadMesh("models/num/1.obj");
	ECS::GetComponent<Transform>(scoreEnt3).SetPosition(glm::vec3(-16.f, 8.5f, 2)).SetScale(2.f).
		SetRotation(glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)));

	score = startingSize;
	step = 0;
	counter = 0;
	betterApple = false;
	dead = false;
	updateHasTail = hasTail;
	updateScore = true;
	paused = false;
	pos = glm::vec2(0);
	velo = glm::vec2(0);
	lastPos = glm::vec2(0, 0);
	targetPos = glm::vec2(0, 0);
	lastTPos = glm::vec2(0, 0);
	targetTPos = glm::vec2(0, 0);

	std::cout << "Tail LERP: " << ((hasTail) ? "ON   \r" : "OFF  \r");
}

glm::vec3 LERP(glm::vec2 p0, glm::vec2 p1, float t) {
	if (t >= 1)	t = 1;
	if (t <= 0)	t = 0;
	glm::vec2 result = (1 - t) * p0 + t * p1;
	return glm::vec3(result.x, result.y, 0.5f);
}

void Snake::Update()
{
	if (updateScore) {
		if (score > 999) score = 999;

		int digit1 = (score - counter) / 100;
		int digit2 = (score - counter) / 10 - digit1 * 10;
		int digit3 = (score - counter) - digit2 * 10 - digit1 * 100;
		if (paused && !dead)	digit1 = digit2 = digit3 = 9;
		ECS::GetComponent<ObjLoader>(scoreEnt1).LoadMesh("models/num/" + std::to_string(digit1) + ".obj");
		ECS::GetComponent<ObjLoader>(scoreEnt2).LoadMesh("models/num/" + std::to_string(digit2) + ".obj");
		ECS::GetComponent<ObjLoader>(scoreEnt3).LoadMesh("models/num/" + std::to_string(digit3) + ".obj");
		updateScore = false;
	}

	if (Input::GetKeyDown(KEY::TAB)) {
		if (ortho = ECS::GetComponent<Camera>(cameraEnt).ToggleOrtho().GetIsOrtho())
			ECS::GetComponent<Transform>(cameraEnt).SetPosition(glm::vec3(-6.75f, -1.75f, 10.f));
		else
			ECS::GetComponent<Transform>(cameraEnt).SetPosition(glm::vec3(-6.75f, -5.f, 26.f));
	}
	
	if (Input::GetKeyDown(KEY::SPACE)) {
		std::cout << "Tail LERP: " << ((updateHasTail = !updateHasTail) ? "ON   \r" : "OFF  \r");
	}

	if (dead) {
		paused = false;
		step += m_dt * (Input::GetKey(KEY::ESC) ? 10 : 3);
		if (updateHasTail != hasTail)
			ECS::GetComponent<Body>(player).HasTail(hasTail = updateHasTail);

		if (counter >= score) {
			lastTPos = targetTPos = targetPos;
			counter += m_dt;
			if (counter - score > 2) {
				std::cout << '\n';
				m_reg = entt::registry();
				Init(BackEnd::GetHalfWidth() * 2, BackEnd::GetHalfHeight() * 2);
				dead = false;
			}
		}
		else while (step >= delay) {
			step -= delay;
			if (counter < score) {
				auto& body = ECS::GetComponent<Body>(player);
				body.Reduce();

				glm::vec2 check = body.GetTail(0) - targetTPos;
				if (abs(check.x) + abs(check.y) > 1) {
					lastTPos = body.GetTail(0);
					targetTPos = body.GetTail();
				}
				else {
					lastTPos = targetTPos;
					targetTPos = body.GetTail();
				}
				if (body.GetSize() <= 1) {
					targetTPos = targetPos;
				}
				check = lastTPos - targetTPos;
				if (abs(check.x) + abs(check.y) > 1) {
					if (check.x != 0)	check.x /= abs(check.x);
					if (check.y != 0)	check.y /= abs(check.y);
					targetTPos = lastTPos + check;
				}
				std::cout << "Snake - Score: " << ++counter << "\r";
				updateScore = true;
			}
		}
		if (hasTail) {
			ECS::GetComponent<Transform>(player).SetPosition(glm::vec3(targetPos, 0.5f));
			ECS::GetComponent<Transform>(tail).SetPosition(LERP(lastTPos, targetTPos, step * delayInv));
		}
		else {
			ECS::GetComponent<Transform>(tail).SetPosition(glm::vec3(targetPos, 0.5f));
		}
	}
	else {
		if (Input::GetKeyDown(KEY::ESC)) {
			paused = !paused;
			updateScore = true;
		}
		if ((velo.x != 0 || velo.y != 0) && !paused)	step += m_dt;

		glm::vec2 input = glm::vec2(0);
		if (Input::GetKey(KEY::W) || Input::GetKey(KEY::UP))	++input.y;
		if (Input::GetKey(KEY::A) || Input::GetKey(KEY::LEFT))	--input.x;
		if (Input::GetKey(KEY::S) || Input::GetKey(KEY::DOWN))	--input.y;
		if (Input::GetKey(KEY::D) || Input::GetKey(KEY::RIGHT))	++input.x;

		if (input.x > 0 && !(pos.x < 0))		velo = glm::vec2(1, 0);
		else if (input.x < 0 && !(pos.x > 0))	velo = glm::vec2(-1, 0);
		else if (input.y > 0 && !(pos.y < 0))	velo = glm::vec2(0, 1);
		else if (input.y < 0 && !(pos.y > 0))	velo = glm::vec2(0, -1);

		//lerping head
		if (hasTail) {
			ECS::GetComponent<Transform>(player).SetPosition(LERP(lastPos, targetPos, step * delayInv));
			ECS::GetComponent<Transform>(tail).SetPosition(LERP(lastTPos, targetTPos, step * delayInv));
		}

		if (Input::GetKey(KEY::LSHIFT))
			if (Input::GetKeyDown(KEY::ESC))
				dead = true;

		if (step >= delay) {
			//reset time using 
			step = 0.f;
			pos = velo;
			if (updateHasTail != hasTail)
				ECS::GetComponent<Body>(player).HasTail(hasTail = updateHasTail);

			auto& pTrans = ECS::GetComponent<Transform>(player);
			auto& body = ECS::GetComponent<Body>(player);

			glm::vec3 newPos = pTrans.GetLocalPosition();
			body.Update(newPos);		//updateBody with currentposition

			newPos += glm::vec3(velo, 0);
			if (newPos.x == BL.x)	newPos.x = TR.x - 1;
			if (newPos.x == TR.x)	newPos.x = BL.x + 1;
			if (newPos.y == BL.x)	newPos.y = TR.y - 1;
			if (newPos.y == TR.x)	newPos.y = BL.y + 1;
			if (hasTail) {
				lastPos = targetPos;
				targetPos = newPos;
				glm::vec2 check = lastPos - targetPos;
				if (abs(check.x) + abs(check.y) > 1) {
					if (check.x != 0)	check.x /= abs(check.x);
					if (check.y != 0)	check.y /= abs(check.y);
					lastPos = targetPos - check;
				}
			}
			else {
				lastPos = targetPos = pTrans.SetPosition(newPos).GetLocalPosition();
			}

			if (body.CheckCollision(newPos) || score == 400) {
				//if die, undo some movement
				newPos -= glm::vec3(velo, 0);

				lastPos = targetPos = pTrans.SetPosition(newPos).GetLocalPosition();
				dead = true;
			}
			else if (newPos == ECS::GetComponent<Transform>(apple).GetLocalPosition()) {
				if (betterApple)		body.AddSegment();	//adds a second time lol
				body.AddSegment();
				score += 1 + betterApple;
				updateScore = true;

				glm::vec3 pos = glm::vec3(rand() % 19 - 9, rand() % 19 - 9, 0.5f);
				while (body.CheckCollision(pos) || pos == newPos) {
					pos = glm::vec3(rand() % 19 - 9, rand() % 19 - 9, 0.5f);
				}
				ECS::GetComponent<Transform>(apple).SetPosition(pos);
				//10% chance
				if (betterApple = !(rand() % 10)) {
					//if it happens, enable animation
					ECS::GetComponent<ObjLoader>(apple).Disable();
					ECS::GetComponent<ObjMorphLoader>(apple).Enable();
					ECS::GetComponent<Transform>(apple).SetScale(1.f);
				}
				else {
					ECS::GetComponent<ObjLoader>(apple).Enable();
					ECS::GetComponent<ObjMorphLoader>(apple).Disable();
					ECS::GetComponent<Transform>(apple).SetScale(0.75f);
				}
			}

			if (hasTail) {
				//for the tail, we need to teleport to body after passing through wall
				glm::vec2 check = body.GetTail(0) - targetTPos;
				if (abs(check.x) + abs(check.y) > 1) {
					lastTPos = body.GetTail(0);
					targetTPos = body.GetTail();
				}
				else {
					lastTPos = targetTPos;
					targetTPos = body.GetTail();
				}

				//if snake teleports through wall, difference is huge
				check = lastTPos - targetTPos;
				if (abs(check.x) + abs(check.y) > 1) {
					if (check.x != 0)	check.x /= abs(check.x);
					if (check.y != 0)	check.y /= abs(check.y);
					targetTPos = lastTPos + check;
				}

			}
			else {
				lastTPos = targetTPos = ECS::GetComponent<Transform>(tail).
					SetPosition(glm::vec3(body.GetTail(), 0.5f)).GetLocalPosition();
			}

		}
	}

	Rendering::LightPos1 = ECS::GetComponent<Transform>(player).GetLocalPosition();
	Rendering::LightPos2 = ECS::GetComponent<Transform>(apple).GetLocalPosition();
}
