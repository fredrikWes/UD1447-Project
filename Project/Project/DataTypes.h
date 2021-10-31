#pragma once
#include "SharedMemory.h"
#include "Graphics.h"
#include "Texture.h"
#include <vector>
#include "Math.h"

struct Node
{
	std::string name = "";

	virtual void Update() {}
};

struct Material : public Node
{
	Vector3 color;
	Texture* texture = nullptr;

	Material(const Message& message)
	{
		name = message.name;
	}

	~Material()
	{
		if (texture)
			delete texture;
	}

	void Update(const MaterialChangedMessage& message)
	{
		if (message.filePathLength != 1)
		{
			if (texture)
				delete texture;
			texture = new Texture(message.filePath);
		}

		else
			color = Vector3(message.color);
	}
};

struct Mesh : public Node
{
	Matrix matrix = Matrix::Identity;

	UINT vertexCount = 0;
	std::vector<Vertex> vertices;
	ID3D11Buffer* vertexBuffer = nullptr;
	static const UINT stride = sizeof(Vertex);
	static const UINT offset = 0;

	std::shared_ptr<Material> material;

	Mesh(const Message& message);
	~Mesh() { if (vertexBuffer) vertexBuffer->Release(); }

	virtual void Update(const MeshChangedMessage& message);
	void Draw();
};