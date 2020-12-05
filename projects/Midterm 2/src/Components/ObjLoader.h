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

#include "Engine/BackEnd.h"
#include "Components/Camera.h"
#include "Engine/Texture2D.h"

class ObjLoader
{
	friend class ObjMorphLoader;
public:
	//contructor
	ObjLoader() {}
	ObjLoader(const std::string& fileName, bool usingMaterial = false);
	~ObjLoader();

	ObjLoader& LoadMesh(const std::string& fileName, bool usingMaterial = false);

	static void Init();
	static void Unload();

	static void BeginDraw();

	void Draw(const glm::mat4& model);
	void DrawSkipQueue(const glm::mat4& model, const glm::mat4& view, const Camera& camera, const glm::vec3& colour,
		const glm::vec3& lightPos1, const glm::vec3& lightPos2, const glm::vec3& lightColour = glm::vec3(1.f),
		float specularStrength = 1.f, float shininess = 4,
		float ambientLightStrength = 0.05f, const glm::vec3& ambientColour = glm::vec3(0.f), float ambientStrength = 0.f
	);

	void Enable() { m_enabled = true; }
	void Disable() { m_enabled = false; }

	static void PerformDraw(const glm::mat4& view, const Camera& camera, const glm::vec3& colour,
		const glm::vec3& lightPos1, const glm::vec3& lightPos2, const glm::vec3& lightColour = glm::vec3(1.f),
		float specularStrength = 1.f, float shininess = 4,
		float ambientLightStrength = 0.05f, const glm::vec3& ambientColour = glm::vec3(0.f), float ambientStrength = 0.f
	);


	struct Texture
	{
		std::string fileName;
		Texture2D::sptr texture;
	};
	static std::vector<Texture> m_textures;

private:

	struct Models
	{
		std::string fileName;
		bool mat;
		bool text = false;
		size_t texture = INT_MAX;
		size_t verts = 0;
		VertexArrayObject::sptr vao = VertexArrayObject::Create();
	};

	struct DrawData
	{
		size_t modelIndex;
		glm::mat4 model;
	};

	static std::vector<DrawData> m_matQueue;
	static std::vector<DrawData> m_texQueue;
	static std::vector<DrawData> m_defaultQueue;
	static std::vector<Models> m_models;
	static Shader::sptr m_shader;
	static Shader::sptr m_matShader;
	static Shader::sptr m_texShader;

	size_t m_index = INT_MAX;
	bool m_enabled = true;
};