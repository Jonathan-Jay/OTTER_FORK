#pragma once

#include "Graphics/Post/PostEffect.h"

class BloomEffect : public PostEffect
{
public:
	//Initializes framebuffer
	//Overrides post effect Init
	void Init(unsigned width, unsigned height) override;

	//Applies the effect to this buffer
	//passes the previous framebuffer with the texture to apply as parameter
	void ApplyEffect(PostEffect* buffer) override;

	void Reshape(unsigned width, unsigned height) override;

	void SetTreshold(float threshold);
	float GetTreshold() const;

	void SetRadius(float radius);
	float GetRadius() const;

	void SetBlurCount(int amt);
	int GetBlurCount() const;

private:
	float _threshold = 0.9f;
	float _radius = 1.f;
	int _blurCount = 5;

	//reduces resolution of the blur buffers
	const float _scalar = 4.f;
};

class ToonEffect : public PostEffect
{
public:
	//Initializes framebuffer
	//Overrides post effect Init
	void Init(unsigned width, unsigned height) override;

	//Applies the effect to this buffer
	//passes the previous framebuffer with the texture to apply as parameter
	void ApplyEffect(PostEffect* buffer) override;

	void SetBands(int amt);
	int GetBands() const;

private:
	int _bands = 5;
};

class PixelEffect : public PostEffect
{
public:
	//Initializes framebuffer
	//Overrides post effect Init
	void Init(unsigned width, unsigned height) override;

	//Applies the effect to this buffer
	//passes the previous framebuffer with the texture to apply as parameter
	void ApplyEffect(PostEffect* buffer) override;

	void Reshape(unsigned width, unsigned height) override;

	void SetPixelCount(int amt);
	int GetPixelCount() const;

private:
	int _pixelCount = 64.f;
};