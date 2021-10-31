#pragma once
#include "DataTypes.h"
#include <map>

class Renderer
{
private:
	//MESHES TO RENDER
	std::map<std::string, std::shared_ptr<Mesh>> meshes;

	//BUFFERS
	ID3D11Buffer* matricesBuffer = nullptr;
	struct Matrices
	{
		Matrix world;
		Matrix viewPerspective;
	} matrices;

	ID3D11Buffer* colorBuffer = nullptr;
	ID3D11Buffer* lightBuffer = nullptr;

	//SHADER PATHS
#ifdef _DEBUG
	const std::string vs_path = "../x64/Debug/VertexShader.cso";
	const std::string ps_path = "../x64/Debug/PixelShader.cso";
	const std::string color_ps_path = "../x64/Debug/ColorPixelShader.cso";
#else
	const std::string vs_path = "../x64/Release/VertexShader.cso";
	const std::string ps_path = "../x64/Release/PixelShader.cso";
	const std::string color_ps_path = "../x64/Release/ColorPixelShader.cso";
#endif

	//SHADERS
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11PixelShader* colorPixelShader = nullptr;

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

		result = Graphics::CreateConstantBuffer(colorBuffer);
		if (!result)
			return;

		result = Graphics::CreateConstantBuffer(lightBuffer);
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

		result = Graphics::CreateShader(colorPixelShader, color_ps_path);
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
		Graphics::BindConstantBuffer(lightBuffer, Shader::PS);
		Graphics::BindConstantBuffer(colorBuffer, Shader::PS, 1);
		Graphics::BindShaders(vertexShader, colorPixelShader);
		Graphics::SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		std::cout << ">>> SUCCEDDED TO INITIALIZE RENDERER <<<" << std::endl;
	}

	~Renderer()
	{
		lightBuffer->Release();
		colorBuffer->Release();
		matricesBuffer->Release();
		vertexShader->Release();
		pixelShader->Release();
		colorPixelShader->Release();
		inputLayout->Release();
	}

	void Render()
	{
		if (meshes.empty())
			return;
		
		for (auto& [name, mesh] : meshes)
		{
			matrices.world = mesh->matrix.Transpose();
			Graphics::UpdateConstantBuffer(matricesBuffer, matrices);

			if (mesh->material)
			{
				if (mesh->material->texture) //TEXTURE
				{
					mesh->material->texture->Bind();
					Graphics::BindShaders(vertexShader, pixelShader);
				}
					
				else //ONLY COLOR
				{
					Graphics::UpdateConstantBuffer(colorBuffer, mesh->material->color);
					Graphics::BindShaders(vertexShader, colorPixelShader);
				}
			}

			else
			{
				Graphics::BindShaders(vertexShader, colorPixelShader);
				Graphics::UpdateConstantBuffer(colorBuffer, Vector3(0.5f, 0.5f, 0.5f)); // DEFAULT COLOR
			}
				
			mesh->Draw();
		}
	}

	void UpdateLightDirection(const Vector3& direction)		{ Graphics::UpdateConstantBuffer(lightBuffer, direction); }
	void UpdateCameraMatrix(const Matrix& matrix)			{ matrices.viewPerspective = matrix; }
	void Bind(std::string name, std::shared_ptr<Mesh> mesh) { meshes[name] = mesh; }
	void Unbind(std::string name)							{ if (meshes.find(name) != meshes.end()) meshes.erase(name); }
};