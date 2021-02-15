#pragma once

#include "Graphics/Framebuffer.h"
#include <Shader.h>

class PostEffect
{
public:
	~PostEffect();

	static void Init(std::string frag);
	static void UnloadShaders();

	static Shader::sptr GetShader(std::string frag);

	//Initialize this effect (will be overriden in each derived class)
	virtual void Init(unsigned width, unsigned height);

	//Applies the effect
	virtual void ApplyEffect(PostEffect* previousBuffer);
	virtual void DrawToScreen();

	//Reshape the buffer
	virtual void Reshape(unsigned width, unsigned height);

	//Clear the buffers
	void Clear();

	//Unloads all the buffers
	virtual void Unload();

	//Binds buffers
	void BindBuffer(int index);
	void UnbindBuffer();

	//Bind Textures
	void BindColorAsTexture(int index, int colorBuffer, int textureSlot);
	void BindDepthAsTexture(int index, int textureSlot);
	void UnbindTexture(int textureSlot);

	//Bind shaders
	void BindShader(int index);
	void UnbindShader();

	void SetInfo(std::string words) { _info = words; }
	std::string Info() { return _info; }

protected:
	std::string _info = "N/A";

	//Holds all our buffers for the effects
	std::vector<Framebuffer*> _buffers;

	//Holds all our shaders for the effects
	std::vector<Shader::sptr> _shaders;

	struct PPShader {
		std::string fragShader;
		Shader::sptr shaderProgram;
	};

	static std::vector<PPShader> shaderCache;
};