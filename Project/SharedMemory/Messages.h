#pragma once
enum class NODETYPE { MESH, DIRECTIONALLIGHT, CAMERA, MATERIAL, TRANSFORM, MATERIALCONNECTION };
enum class MESSAGETYPE { ADDED, REMOVED, CHANGED, NAMECHANGE };

//BEHÖVS EJ?
//enum class ATTRIBUTETYPE { GEOMETRY, TEXTURE, COLOR };

//------CASES------
//MESH ADDED
//MESH REMOVED
//MESH CHANGED

//TRANSFORM CHANGED

//DIRECTIONALLIGHT ADDED
//DIRECTIONALLIGHT REMOVED
//DIRECTIONALLIGHT CHANGED

//CAMERA ADDED
//CAMERA REMOVED
//CAMERA CHANGED

//MATERIAL ADDED
//MATERIAL REMOVED
//MATERIAL CHANGED

struct Vertex
{
	float Px, Py, Pz;
	float nX, nY, nZ;
	float u, v;

	bool operator==(const Vertex& other)
	{
		if (Px == other.Px && Py == other.Py && Pz == other.Pz &&
			nX == other.nX && nY == other.nY && nZ == other.nZ &&
			u == other.u && v == other.v)
			return true;

		return false;
	}
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

struct CameraChangedMessage : public Message
{
	float viewMatrix[16];
	bool orthographic = false;
	double orthoWidth;

	CameraChangedMessage(size_t nameLength, char* name, float matrix[], double orthoWidth, bool orthographic)
		:Message(NODETYPE::CAMERA, MESSAGETYPE::CHANGED, nameLength, name), orthographic(orthographic)
	{
		this->orthoWidth = orthoWidth;

		for (UINT i = 0; i < 16; ++i)
			this->viewMatrix[i] = matrix[i];
	}

	CameraChangedMessage(char* data)
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

		memcpy(viewMatrix, data + location, sizeof(float) * 16);
		location += sizeof(float) * 16;

		memcpy(&orthoWidth, data + location, sizeof(double));
		location += sizeof(double);

		memcpy(&orthographic, data + location, sizeof(bool));

	}

	virtual void* Data() override
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

		memcpy(data + location, viewMatrix, sizeof(float) * 16);
		location += sizeof(float) * 16;

		memcpy(data + location, &orthoWidth, sizeof(double));
		location += sizeof(double);

		memcpy(data + location, &orthographic, sizeof(bool));

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + sizeof(float) * 16 + sizeof(double) + sizeof(bool); return messageSize; }
};

struct MeshChangedMessage : public Message
{
	int numVertices;
	Vertex* vertices;

	MeshChangedMessage(NODETYPE nodeType, MESSAGETYPE messageType, size_t nameLength, char* name, Vertex* vertices, int numVertices)
		:Message(nodeType, messageType, nameLength, name), numVertices(numVertices)
	{
		this->vertices = new Vertex[numVertices];
		memcpy(this->vertices, vertices, sizeof(Vertex) * numVertices);
	}

	~MeshChangedMessage() { delete vertices; }

	MeshChangedMessage(char* data)
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

		memcpy(&numVertices, data + location, sizeof(int));
		location += sizeof(int);

		vertices = new Vertex[numVertices];
		memcpy(vertices, data + location, sizeof(Vertex) * numVertices);
		location += sizeof(Vertex) * numVertices;
	}

	virtual void* Data() override
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

		memcpy(data + location, &numVertices, sizeof(int));
		location += sizeof(int);

		memcpy(data + location, vertices, sizeof(Vertex) * numVertices);
		location += sizeof(Vertex) * numVertices;

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + sizeof(int) + sizeof(Vertex) * numVertices; return messageSize; }
};