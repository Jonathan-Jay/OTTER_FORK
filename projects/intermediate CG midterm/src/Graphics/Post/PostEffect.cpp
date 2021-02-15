#include "PostEffect.h"

std::vector<PostEffect::PPShader> PostEffect::shaderCache = {};

PostEffect::~PostEffect()
{
	Unload();
}

void PostEffect::Init(std::string frag)
{
	if (GetShader(frag) != nullptr) return;
	int index = int(shaderCache.size());
	shaderCache.push_back({ frag, Shader::Create() });
	shaderCache[index].shaderProgram->LoadShaderPartFromFile("shaders/Post/passthrough_vert.glsl", GL_VERTEX_SHADER);
	shaderCache[index].shaderProgram->LoadShaderPartFromFile(frag.c_str(), GL_FRAGMENT_SHADER);
	shaderCache[index].shaderProgram->Link();
}

void PostEffect::UnloadShaders()
{
	shaderCache.clear();
}

Shader::sptr PostEffect::GetShader(std::string frag)
{
	for (int i(0); i < shaderCache.size(); ++i) {
		if (shaderCache[i].fragShader == frag)
			return shaderCache[i].shaderProgram;
	}
	return nullptr;
}

void PostEffect::Init(unsigned width, unsigned height)
{
	//Set up the framebuffers
	int index = int(_buffers.size());
	if (index == 0) {
		_buffers.push_back(new Framebuffer());
		_buffers[index]->AddColorTarget(GL_RGBA8);
		_buffers[index]->AddDepthTarget();
		_buffers[index]->Init(width, height);
	}

	index = int(_shaders.size());
	if (index == 0) {
		_shaders.push_back(GetShader("shaders/Post/passthrough_frag.glsl"));
	}
}

void PostEffect::ApplyEffect(PostEffect* previousBuffer)
{
	BindShader(0);

	previousBuffer->BindColorAsTexture(0, 0, 0);

	_buffers[0]->RenderToFSQ();

	previousBuffer->UnbindTexture(0);

	UnbindShader();
}

void PostEffect::DrawToScreen()
{
	PostEffect::BindShader(0);

	BindColorAsTexture(0, 0, 0);

	_buffers[0]->DrawFullscreenQuad();

	UnbindTexture(0);

	PostEffect::UnbindShader();
}

void PostEffect::Reshape(unsigned width, unsigned height)
{
	for (unsigned int i = 0; i < _buffers.size(); i++)
	{
		_buffers[i]->Reshape(width, height);
	}
}

void PostEffect::Clear()
{
	for (unsigned int i = 0; i < _buffers.size(); i++)
	{
		_buffers[i]->Clear();
	}
}

void PostEffect::Unload()
{
	for (unsigned int i = 0; i < _buffers.size(); i++)
	{
		if (_buffers[i] != nullptr)
		{
			_buffers[i]->Unload();
			delete _buffers[i];
			_buffers[i] = nullptr;
		}
	}
	_buffers.clear();

	//_shaders.clear();
}

void PostEffect::BindBuffer(int index)
{
	_buffers[index]->Bind();
}

void PostEffect::UnbindBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void PostEffect::BindColorAsTexture(int index, int colorBuffer, int textureSlot)
{
	_buffers[index]->BindColorAsTexture(colorBuffer, textureSlot);
}

void PostEffect::BindDepthAsTexture(int index, int textureSlot)
{
	_buffers[index]->BindDepthAsTexture(textureSlot);
}

void PostEffect::UnbindTexture(int textureSlot)
{
	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void PostEffect::BindShader(int index)
{
	_shaders[index]->Bind();
}

void PostEffect::UnbindShader()
{
	glUseProgram(GL_NONE);
}
