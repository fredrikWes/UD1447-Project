#pragma once
enum class NODETYPE { MESH, POINTLIGHT, CAMERA, MATERIAL };
enum class MESSAGETYPE { ADDED, REMOVED, CHANGED };
enum class ATTRIBUTETYPE { GEOMETRY, TRANSFORM, TEXTURE, COLOR };

struct Vertex
{
	float Px, Py, Pz;
	float nX, nY, nZ;
	float u, v;
};

struct Message
{
	size_t messageSize;
	NODETYPE nodeType;
	MESSAGETYPE messageType;
	size_t nameLength;
	char* name;

	Message() = default;

	Message(NODETYPE nodeType, MESSAGETYPE messageType, size_t nameLength, char* name)
		: nodeType(nodeType), messageType(messageType), nameLength(nameLength + 1), name(name), messageSize(0) {}

	Message(char* data)
	{
		size_t location = 0;

		memcpy(&messageSize, data + location, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(&nodeType, data + location, sizeof(NODETYPE));
		location += sizeof(NODETYPE);

		memcpy(&messageType, data + location, sizeof(MESSAGETYPE));
		location += sizeof(MESSAGETYPE);

		memcpy(&nameLength, data + location, sizeof(size_t));
		location += sizeof(size_t);

		name = new char[nameLength];
		strcpy_s(name, nameLength, data + location);
	}

	virtual void* Data() 
	{ 
		size_t location = 0;
		char* data = new char[Size()];

		memcpy(data, &messageSize, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(data + location, &nodeType, sizeof(NODETYPE));
		location += sizeof(NODETYPE);

		memcpy(data + location, &messageType, sizeof(MESSAGETYPE));
		location += sizeof(MESSAGETYPE);

		memcpy(data + location, &nameLength, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(data + location, name, nameLength);

		return data;
	};

	virtual size_t Size() { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength; return messageSize; };
};

/*
struct NodeAddedMessage : public Message
{
	NodeAddedMessage(char* data)
	{
		memcpy(&messageSize, data, sizeof(size_t));
		memcpy(&type, data + sizeof(size_t), sizeof(NODETYPE));
		memcpy(&nameLength, data + sizeof(size_t) + sizeof(NODETYPE), sizeof(size_t));
		name = new char[nameLength];
		strcpy_s(name, nameLength, data + sizeof(size_t) + sizeof(NODETYPE) + sizeof(size_t));
	}

	NodeAddedMessage(NODETYPE type, char* name, size_t nameLength) : nameLength(nameLength + 1) { this->messageSize = 0; this->name = name; this->type = type; }

	virtual void* Data() override
	{
		size_t location = 0;
		char* data = new char[Size()];

		memcpy(data, &messageSize, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(data + location, &type, sizeof(NODETYPE));
		location += sizeof(NODETYPE);

		memcpy(data + location, &nameLength, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(data + location, name, nameLength);

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + nameLength; return messageSize; }
};
*/

struct OutMeshChangedMessage : public Message
{
	ATTRIBUTETYPE attributeType;

	int numVertices;
	Vertex* vertices;

	int numIndices;
	int* indices;

	OutMeshChangedMessage(NODETYPE nodeType, MESSAGETYPE messageType, size_t nameLength, char* name, ATTRIBUTETYPE attributeType, Vertex* vertices, int numVertices, int* indices, int numIndices)
		:Message(nodeType, messageType, nameLength, name), attributeType(attributeType), numVertices(numVertices), numIndices(numIndices), vertices(vertices), indices(indices) {}

	~OutMeshChangedMessage()
	{
		if (vertices)
			delete vertices;
		if (indices)
			delete indices;
	}

	OutMeshChangedMessage(char* data)
	{
		size_t location = 0;

		memcpy(&messageSize, data + location, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(&nodeType, data + location, sizeof(NODETYPE));
		location += sizeof(NODETYPE);

		memcpy(&messageType, data + location, sizeof(MESSAGETYPE));
		location += sizeof(MESSAGETYPE);

		memcpy(&nameLength, data + location, sizeof(size_t));
		location += sizeof(size_t);

		name = new char[nameLength];
		strcpy_s(name, nameLength, data + location);
		location += nameLength;

		memcpy(&attributeType, data + location, sizeof(ATTRIBUTETYPE));
		location += sizeof(ATTRIBUTETYPE);

		memcpy(&numVertices, data + location, sizeof(int));
		location += sizeof(int);

		vertices = new Vertex[numVertices];
		memcpy(&vertices, data + location, sizeof(Vertex) * numVertices);
		location += sizeof(Vertex) * numVertices;

		memcpy(&numIndices, data + location, sizeof(int));
		location += sizeof(int);

		indices = new int[numIndices];
		memcpy(&indices, data + location, sizeof(int) * numIndices);
	}

	virtual void* Data()
	{
		size_t location = 0;
		char* data = new char[Size()];

		memcpy(data, &messageSize, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(data + location, &nodeType, sizeof(NODETYPE));
		location += sizeof(NODETYPE);

		memcpy(data + location, &messageType, sizeof(MESSAGETYPE));
		location += sizeof(MESSAGETYPE);

		memcpy(data + location, &nameLength, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(data + location, name, nameLength);
		location += nameLength;

		memcpy(data, &numVertices, sizeof(int));
		location += sizeof(int);

		memcpy(data + location, vertices, sizeof(Vertex) * numVertices);
		location += sizeof(Vertex) * numVertices;

		memcpy(data + location, &numIndices, sizeof(int));
		location += sizeof(int);

		memcpy(data + location, indices, sizeof(int) * numIndices);

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + sizeof(int) * 2 + sizeof(Vertex) * numVertices + sizeof(int) * numIndices; return messageSize; }
};