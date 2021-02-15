#include "SepiaEffect.h"

void SepiaEffect::Init(unsigned width, unsigned height)
{
	int index = int(_buffers.size());
	if (index == 0) {
		_buffers.push_back(new Framebuffer());
		_buffers[index]->AddColorTarget(GL_RGBA8);
		_buffers[index]->Init(width, height);
	}

	//Loads the shaders
	index = int(_shaders.size());
	if (index == 0) {
		_shaders.push_back(GetShader("shaders/Post/passthrough_frag.glsl"));
		_shaders.push_back(GetShader("shaders/Post/sepia_frag.glsl"));
	}
}

void SepiaEffect::ApplyEffect(PostEffect* buffer)
{
	BindShader(1);
	_shaders[1]->SetUniform("u_Intensity", _intensity);

	buffer->BindColorAsTexture(0, 0, 0);

	_buffers[0]->RenderToFSQ();

	buffer->UnbindTexture(0);

	UnbindShader();
}

float SepiaEffect::GetIntensity() const
{
	return _intensity;
}

void SepiaEffect::SetIntensity(float intensity)
{
	_intensity = intensity;
}
