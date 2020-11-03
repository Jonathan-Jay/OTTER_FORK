#pragma once
#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include "dependencies/entt.hpp"
#include "BackEndThings/VertexArrayObject.h"
#include "BackEndThings/IndexBuffer.h"
#include "BackEndThings/Shader.h"

class BackEnd abstract
{
public:
	//returns
	static GLFWwindow* Init(std::string name);

private:
	static GLFWwindow* window;
};