#pragma once
#include "DataTypes.h"
#include <map>

class Renderer
{
private:
	//MESHES TO RENDER
	std::map<std::string, std::shared_ptr<Mesh>> meshes;

	//BUFFER(S)
	ID3D11Buffer* matricesBuffer = nullptr;
	struct Matrices
	{
		Matrix world;
		Matrix viewPerspective;
	} matrices;

	//SHADER PATHS
#ifdef _DEBUG
	const std::string vs_path = "";
	const std::string ps_path = "";
#else
	const std::string vs_path = "";
	const std::string ps_path = "";
#endif

	//SHADERS
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;

	//INPUT LAYOUT
	ID3D11InputLayout* inputLayout = nullptr;

public:
	Renderer()
	{
		bool result = true;

		//BUFFER(S)
		result = Graphics::CreateConstantBuffer(matricesBuffer, sizeof(Matrices));
		if (!result)
			return;

		//SHADERS
		std::string byteCode;
		result = Graphics::CreateShader(vertexShader, vs_path, byteCode);
		if (!result)
			return;

		result = Graphics::CreateShader(pixelShader, ps_path);
		if (!result)
			return;

		//INPUT LAYOUT
		D3D11_INPUT_ELEMENT_DESC inputDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"UVS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		result = Graphics::CreateInputLayout(inputLayout, inputDesc, ARRAYSIZE(inputDesc), byteCode);
		if (!result)
			return;

		//BINDINGS
		Graphics::BindInputLayout(inputLayout);
		Graphics::BindConstantBuffer(matricesBuffer);
		Graphics::SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Graphics::BindShaders(vertexShader, pixelShader);

		std::cout << ">>> SUCCEDDED TO INITIALIZE RENDERER <<<" << std::endl;
	}

	~Renderer()
	{
		matricesBuffer->Release();
		vertexShader->Release();
		pixelShader->Release();
		inputLayout->Release();
	}

	void Render()
	{
		if (meshes.empty())
			return;
		
		for (auto& [name, mesh] : meshes)
		{
			matrices.world = mesh->matrix;
			Graphics::UpdateConstantBuffer(matricesBuffer, matrices);

			mesh->Draw();
		}
	}

	void UpdateCameraMatrix(const Matrix& matrix)			{ matrices.viewPerspective = matrix; }
	void Bind(std::string name, std::shared_ptr<Mesh> mesh) { meshes[name] = mesh; }
	void Unbind(std::string name)							{ if (meshes.find(name) != meshes.end()) meshes.erase(name); }
};