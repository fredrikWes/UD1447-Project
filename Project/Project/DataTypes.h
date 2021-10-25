#pragma once
#include "SharedMemory.h"
#include "Graphics.h"
#include "Texture.h"
#include <vector>

struct Node
{
	std::string name = "";
	virtual void Update() {}
};

struct Mesh : public Node
{
	Matrix matrix = Matrix::Identity;

	UINT vertexCount = 0;
	std::vector<Vertex> vertices;
	ID3D11Buffer* vertexBuffer = nullptr;
	static const UINT stride = sizeof(Vertex);
	static const UINT offset = 0;

	//MATERIAL

	Mesh(const Message& message);
	~Mesh() { if (vertexBuffer) vertexBuffer->Release(); }

	virtual void Update(const MeshChangedMessage& message);
	void Draw();
};