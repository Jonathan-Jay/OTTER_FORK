#pragma once
#include "Scene.h"

//holds all inits and unloads
namespace Gameloop
{
	//init everything and return the window from backend
	GLFWwindow* Start(const std::string& name, int width, int height) {
		srand(time(0));
		// We'll borrow the logger from the toolkit, but we need to initialize it
		Logger::Init();

		GLFWwindow* window = BackEnd::Init("Mecha Mayhem", width, height);
		if (!window)	return nullptr;

		//all static inits
		ObjLoader::Init();
		ObjMorphLoader::Init();
		Input::Init(window);

		return window;
	}

	//update anything with global updates
	void Update() {
		Input::Update();
	}

	//stop everything (use at the end to avoid issues)
	void Stop() {
		//unloading static things
		Input::Unload();
		ECS::DettachRegistry();
		BackEnd::Unload();
		ObjLoader::Unload();
		ObjMorphLoader::Unload();

		// Clean up the toolkit logger so we don't leak memory
		Logger::Uninitialize();
	}
}