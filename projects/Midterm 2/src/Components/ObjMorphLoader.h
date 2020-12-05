#pragma once
#include "ObjLoader.h"

class ObjMorphLoader
{
public:
	//contructor
	ObjMorphLoader() {}
	ObjMorphLoader(const std::string& baseFileName, bool usingMaterial = false);
	~ObjMorphLoader();

	ObjMorphLoader& LoadMeshs(const std::string& baseFileName, bool usingMaterial = false);

	static void Init();
	static void Unload();
	
	ObjMorphLoader& SetDirection(bool forwards) {
		if (m_reversing == forwards)
			ToggleDirection();
		return *this;
	}

	//returns true if moving forwards
	bool GetDirection() { return !m_reversing; }

	ObjMorphLoader& ToggleDirection();

	ObjMorphLoader& SetBounce(bool bounce) {
		m_bounce = bounce;
		return *this;
	}

	bool GetBounce() { return m_bounce; }

	ObjMorphLoader& ToggleBounce() {
		m_bounce = !m_bounce;
		return *this;
	}

	ObjMorphLoader& SetLooping(bool loop) {
		m_loop = loop;
		return *this;
	}

	bool GetLooping() { return m_loop; }

	ObjMorphLoader& ToggleLooping() {
		m_loop = !m_loop;
		return *this;
	}

	ObjMorphLoader& SetSpeed(float speed) { m_speed = speed; return *this; }
	float GetSpeed() { return m_speed; }

	static void BeginDraw();

	void Update(float dt);

	void Draw(const glm::mat4& model);

	void Enable() { m_enabled = true; }
	void Disable() { m_enabled = false; }

	static void PerformDraw(const glm::mat4& view, const Camera& camera, const glm::vec3& colour,
		const glm::vec3& lightPos1, const glm::vec3& lightPos2, const glm::vec3& lightColour = glm::vec3(1.f),
		float specularStrength = 1.f, float shininess = 4,
		float ambientLightStrength = 0.05f, const glm::vec3& ambientColour = glm::vec3(0.f), float ambientStrength = 0.f
	);

private:
	struct Frames {
		VertexBuffer::sptr pos;
		VertexBuffer::sptr normal;
		VertexBuffer::sptr colour = nullptr;
		VertexBuffer::sptr spec = nullptr;
	};

	struct Animations {
		std::string fileName;
		bool mat;
		bool text = false;
		bool bounce = false;
		bool loop = false;
		bool reversing = false;
		float startTime = 0;
		std::vector<float> durations = {};
		std::vector<size_t> frameIndices = {};
		std::vector<Frames> frames = {};
		size_t start = 0;
		size_t t = 0;
		size_t texture = INT_MAX;
		VertexBuffer::sptr UVs = nullptr;
	};

	struct DrawData {
		float t;
		VertexArrayObject::sptr vao;
		glm::mat4 model;
		size_t texture = 0;
	};

	static std::vector<DrawData> m_texQueue;
	static std::vector<DrawData> m_matQueue;
	static std::vector<DrawData> m_defaultQueue;
	static std::vector<Animations> m_anims;
	static Shader::sptr m_shader;
	static Shader::sptr m_matShader;
	static Shader::sptr m_texShader;
	static std::vector<BufferAttribute> pos1Buff;
	static std::vector<BufferAttribute> pos2Buff;
	static std::vector<BufferAttribute> norm1Buff;
	static std::vector<BufferAttribute> norm2Buff;
	static std::vector<BufferAttribute> col1Buff;
	static std::vector<BufferAttribute> col2Buff;
	static std::vector<BufferAttribute> spec1Buff;
	static std::vector<BufferAttribute> spec2Buff;
	static std::vector<BufferAttribute> UVBuff;

	VertexArrayObject::sptr m_vao = nullptr;

	float m_timer = 0;
	float m_t = 0;
	float m_speed = 1;

	size_t m_p0 = 0;
	size_t m_p1 = 0;
	size_t m_index = INT_MAX;

	bool m_reversing = false;
	bool m_bounce = false;
	bool m_loop = false;
	bool m_enabled = true;
};