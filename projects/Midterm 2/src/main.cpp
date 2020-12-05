#include "Utilities/Gameloop.h"
#include "Tank.h"
#include "Snake.h"

int main() {
	int width = 1550, height = 750, width2 = 1280, height2 = 720;
	GLFWwindow* window = Gameloop::Start("NJN", width, height);
	if (!window)	return 1;

	{
		//preloading meshes
		ObjLoader("models/explosion.obj", true).LoadMesh("models/Ball.obj").
			LoadMesh("models/num/9.obj").LoadMesh("models/num/8.obj").LoadMesh("models/num/7.obj").
			LoadMesh("models/num/6.obj").LoadMesh("models/num/5.obj").LoadMesh("models/num/4.obj").
			LoadMesh("models/num/3.obj").LoadMesh("models/num/2.obj").LoadMesh("models/num/1.obj").
			LoadMesh("models/num/0.obj").LoadMesh("models/Cube.obj").LoadMesh("models/Pillar.obj");

		std::cout << "\nCollection of games by Jonathan Jay - 100743575\n"
			"Click LCTRL and ENTER at together to swap scenes\n"
			"\nBattle Tank\n"
			"Controls are as follows:\n"
			"\t\tP1 (RED):\t\tP2 (BLUE):\n"
			"\t-Move:\tWASD\t\t\tIJKL\n"
			"\t-Shoot:\tLeftShift or C\t\tN or .\n\n"
			"\tGlobal:\n"
			"\t-Tab to toggle persective\n"
			"\t-SpaceBar to toggle screen shake\n"
			"\t-ESC to reset\n"
			"Whenever the game is reset, it switches between a set map and a randomized map\n"
			"\nSnake\n"
			"Controls are as follows:\n"
			"\t-WASD or arrow keys to move\n"
			"\t-Tab to toggle perspective\n"
			"\t-SpaceBar to toggle tail LERP\n"
			"\t-press ESC to pause (score will change to 999) and hold speed up death anim\n"
			"\t-LSHIFT + ESC to reset\n"
			"Eat apples to grow, the morphing apple makes you grow twice\n"
			"walls don't kill this time, but take you to the other side\n"
			"LERP ON makes the snake look shorter, that's because of how it looks when animated!\n"
			"\n--Game Outputs--\n";


		// Creating demo scene
		std::vector<Scene*> scenes;
		//scenes.push_back(new Frogger("Frogger"));
		//scenes.push_back(new Tank("Tank"));
		scenes.push_back(new Tank("Battle Tank"));
		scenes.push_back(new Snake("Snake"));

		scenes[1]->Init(width2, height2);
		scenes[0]->Init(width, height);

		bool sceneswap = false;
		//bool sceneswap = true;
		Scene* activeScene = scenes[sceneswap]->Reattach();
		glfwSetWindowTitle(window, activeScene->GetName().c_str());

		Rendering::massDraw = false;

		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			if (Input::GetKey(KEY::LCTRL) && Input::GetKeyUp(KEY::ENTER)) {
				activeScene->Exit();
				glfwSetWindowTitle(window, (activeScene = scenes[sceneswap = !sceneswap]->Reattach())->GetName().c_str());

				if (sceneswap) {	// [1] aka snake
					BackEnd::SetAspect(width2, height2);
					Rendering::massDraw = true;
					Rendering::BackColour = glm::vec4(0.25f, 0.75f, 0.25f, 1.f);
					Rendering::LightColour = glm::vec3(1.f);
					Rendering::DefaultColour = glm::vec3(0.f, 0.5f, 0.f);
				}
				else {				// [0] aka tank
					BackEnd::SetAspect(width, height);
					Rendering::massDraw = false;
					Rendering::BackColour = glm::vec4(0.2f, 0.2f, 0.2f, 1.f);
					Rendering::LightColour = glm::vec3(3.f);
				}
			}


			activeScene->Update();


			//do not touch plz
			activeScene->BackEndUpdate();
			Gameloop::Update();

			glfwSwapBuffers(window);
		}

		activeScene->Exit();
		std::cout << "\n\nThanks for playing\n\n";
	}

	Gameloop::Stop();
	return 0;
}