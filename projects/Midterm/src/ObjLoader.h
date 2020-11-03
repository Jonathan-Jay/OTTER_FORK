#pragma once
#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <GLM/glm.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "BackEndThings/BackEnd.h"
#include "Camera.h"
#include "Texture2D.h"
#include "Texture2DData.h"

struct Models
{
	std::string fileName;
	bool mat;
	bool text = false;
	size_t texture = 0;
	std::vector<VertexArrayObject::sptr> vao = {};
	std::vector<size_t> verts = {};
};

struct Texture
{
	std::string fileName;
	Texture2D::sptr texture;
};

class ObjLoader
{
public:
	//contructor
	ObjLoader() {}
	ObjLoader(const std::string& fileName, bool usingMaterial = false);
	~ObjLoader();

	static void Init();

	ObjLoader& LoadMesh(const std::string& fileName, bool usingMaterial = false);

	void Draw(Camera camera, glm::mat4 model, glm::mat3 rotation, glm::vec3 colour,
		glm::vec3 lightPos1, glm::vec3 lightPos2, glm::vec3 lightColour = glm::vec3(1.f), float specularStrength = 1.f, float shininess = 4,
		float ambientLightStrength = 0.05f, glm::vec3 ambientColour = glm::vec3(0.f), float ambientStrength = 0.1f
	);

	Shader::sptr GetShader() { return m_shader; }

private:
	static Shader::sptr m_shader;
	static Shader::sptr m_matShader;
	static Shader::sptr m_texShader;
	static std::vector<Models> m_models;
	static std::vector<Texture> m_textures;

	size_t m_index = INT_MAX;
	size_t m_texture = INT_MAX;
	bool m_usingMaterial = false;
	bool m_usingTexture = false;
};