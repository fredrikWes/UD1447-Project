#pragma once
#include "SharedMemory.h"
#include "Graphics.h"
#include "Texture.h"
#include <vector>

struct Node
{
	std::string name = "";
};

struct Mesh : public Node
{
	Matrix matrix = Matrix::Identity;

	UINT vertexCount = 0;
	//std::vector<Vertex> vertices;
	ID3D11Buffer* vertexBuffer = nullptr;
	static const UINT stride = sizeof(Vertex);

	//MATERIAL

	Mesh(const Message& message);
	void Update(const MeshChangedMessage& message);
	void Draw();
};