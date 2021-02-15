#pragma once

#include "Graphics/Post/PostEffect.h"
#include "Graphics/LUT.h"

class ColourCorrection : public PostEffect
{
public:
	//Initializes framebuffer
	//Overrides post effect Init
	void Init(unsigned width, unsigned height) override;

	//Applies the effect to this buffer
	//passes the previous framebuffer with the texture to apply as parameter
	void ApplyEffect(PostEffect* buffer) override;

	void Unload() override;

	void SetCube(LUT3D &cube);
	LUT3D GetCube() const;

private:
	LUT3D *_cube;
};