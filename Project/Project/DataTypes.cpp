#include "DataTypes.h"

Mesh::Mesh(const Message& message)
	:vertexCount(0), matrix(Matrix::Identity)
{ 
	name = std::string(message.name); 
}

void Mesh::Update(const MeshChangedMessage& message)
{
	vertexCount = message.numVertices;

	//for (UINT i = 0; i < vertexCount; ++i)
	//	vertices.emplace_back(message.vertices[i]);

	if (vertexBuffer)
		vertexBuffer->Release();

	Graphics::CreateVertexBuffer(vertexBuffer, sizeof(Vertex), vertexCount, message.vertices);
}

void Mesh::Draw()
{
	if (!vertexBuffer)
		return;

	Graphics::BindVertexBuffer(vertexBuffer, &Mesh::stride);
}