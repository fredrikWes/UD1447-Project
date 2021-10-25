#include "DataTypes.h"

Mesh::Mesh(const Message& message)
	:vertexCount(0)
{ 
	name = std::string(message.name); 
}

void Mesh::Update(const MeshChangedMessage& message)
{
	vertexCount = message.vertexCount;
	vertices.resize(vertexCount, Vertex());

	for (UINT i = 0; i < message.numIndices; ++i)
	{
		std::cout << "CHANGED VERTEX: " << message.indices[i] << std::endl;
		vertices[message.indices[i]] = message.vertices[i];
	}
		
	if (vertexBuffer)
		vertexBuffer->Release();

	Graphics::CreateVertexBuffer(vertexBuffer, sizeof(Vertex), vertexCount, message.vertices);
}

void Mesh::Draw()
{
	if (!vertexBuffer)
		return;

	Graphics::BindVertexBuffer(vertexBuffer, &Mesh::stride, &Mesh::offset);
	Graphics::Draw(vertexCount);
}