#pragma once
#include "Graphics.h"

class Texture
{
private:
	ID3D11ShaderResourceView* srv = nullptr;
public:
	Texture() = default;
	~Texture() { srv->Release(); }
	Texture(const std::string& path);

	void Bind() const
	{
		Graphics::BindShaderResourceView(srv);
	}
};