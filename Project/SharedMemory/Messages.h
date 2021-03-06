#pragma once
enum class NODETYPE { MESH, CAMERA, MATERIAL, TRANSFORM, MATERIALCONNECTION, NAMECHANGE };
enum class MESSAGETYPE { ADDED, REMOVED, CHANGED };

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
	bool orthographic = false;
	double orthoWidth;
	float nearZ;
	float farZ;
	float horFOV;
	float verFOV;
	float eyePos[4];
	float center[4];
	double up[3];
	int portWidth;
	int portHeight;

	CameraChangedMessage(size_t nameLength, char* name, double orthoWidth, bool orthographic, float nearZ, 
						 float farZ, float horFOV, float verFOV, float eyePos[4], float center[4], double up[3],
						 int portWidth, int portHeight)
		:Message(NODETYPE::CAMERA, MESSAGETYPE::CHANGED, nameLength, name), orthographic(orthographic), nearZ(nearZ), farZ(farZ),
				 horFOV(horFOV), verFOV(verFOV), portWidth(portWidth), portHeight(portHeight)
	{
		this->orthoWidth = orthoWidth;

		for (UINT i = 0; i < 4; ++i)
		{
			this->eyePos[i] = eyePos[i];
			this->center[i] = center[i];
		}

		for (UINT i = 0; i < 3; ++i)
		{
			this->up[i] = up[i];
		}	
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

		memcpy(&orthoWidth, data + location, sizeof(double));
		location += sizeof(double);

		memcpy(&orthographic, data + location, sizeof(bool));
		location += sizeof(bool);

		memcpy(&nearZ, data + location, sizeof(float));
		location += sizeof(float);

		memcpy(&farZ, data + location, sizeof(float));
		location += sizeof(float);

		memcpy(&horFOV, data + location, sizeof(float));
		location += sizeof(float);

		memcpy(&verFOV, data + location, sizeof(float));
		location += sizeof(float);

		memcpy(eyePos, data + location, sizeof(float) * 4);
		location += sizeof(float) * 4;

		memcpy(center, data + location, sizeof(float) * 4);
		location += sizeof(float) * 4;

		memcpy(up, data + location, sizeof(double) * 3);
		location += sizeof(double) * 3;

		memcpy(&portWidth, data + location, sizeof(int));
		location += sizeof(int);

		memcpy(&portHeight, data + location, sizeof(int));
		location += sizeof(int);

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

		memcpy(data + location, &orthoWidth, sizeof(double));
		location += sizeof(double);

		memcpy(data + location, &orthographic, sizeof(bool));
		location += sizeof(bool);

		memcpy(data + location ,&nearZ, sizeof(float));
		location += sizeof(float);

		memcpy( data + location, &farZ, sizeof(float));
		location += sizeof(float);

		memcpy(data + location, &horFOV, sizeof(float));
		location += sizeof(float);

		memcpy(data + location, &verFOV, sizeof(float));
		location += sizeof(float);

		memcpy(data + location, eyePos, sizeof(float) * 4);
		location += sizeof(float) * 4;

		memcpy(data + location, center, sizeof(float) * 4);
		location += sizeof(float) * 4;

		memcpy(data + location, up, sizeof(double) * 3);
		location += sizeof(double) * 3;

		memcpy(data + location, &portWidth, sizeof(int));
		location += sizeof(int);

		memcpy(data + location, &portHeight, sizeof(int));
		location += sizeof(int);

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + sizeof(double) + sizeof(bool) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) * 4 + sizeof(float) * 4 + sizeof(double) * 3 + sizeof(int) + sizeof(int); return messageSize; }
};

struct MeshChangedMessage : public Message
{
	int numIndices;
	int* indices;

	int numVertices;
	Vertex* vertices;

	MeshChangedMessage(NODETYPE nodeType, MESSAGETYPE messageType, size_t nameLength, char* name, int* indices, int numIndices, Vertex* vertices, int numVertices)
		:Message(nodeType, messageType, nameLength, name), numIndices(numIndices), numVertices(numVertices)
	{
		this->vertices = new Vertex[numVertices];
		memcpy(this->vertices, vertices, sizeof(Vertex) * numVertices);

		this->indices = new int[numIndices];
		memcpy(this->indices, indices, sizeof(int) * numIndices);
	}

	~MeshChangedMessage() { delete vertices; delete indices; }

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

		memcpy(&numIndices, data + location, sizeof(int));
		location += sizeof(int);

		indices = new int[numIndices];
		memcpy(indices, data + location, sizeof(int) * numIndices);
		location += sizeof(int) * numIndices;

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

		memcpy(data + location, &numIndices, sizeof(int));
		location += sizeof(int);

		memcpy(data + location, indices, sizeof(int) * numIndices);
		location += sizeof(int) * numIndices;

		memcpy(data + location, &numVertices, sizeof(int));
		location += sizeof(int);

		memcpy(data + location, vertices, sizeof(Vertex) * numVertices);
		location += sizeof(Vertex) * numVertices;

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + sizeof(int) * 2 + sizeof(int) * numIndices + sizeof(Vertex) * numVertices; return messageSize; }
};

struct TransformChangedMessage : public Message
{
	float matrix[16];

	TransformChangedMessage(size_t nameLength, char* name, float matrix[16])
		:Message(NODETYPE::TRANSFORM, MESSAGETYPE::CHANGED, nameLength, name)
	{
		memcpy(this->matrix, matrix, sizeof(float) * 16);
	}

	TransformChangedMessage(char* data)
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

		memcpy(matrix, data + location, sizeof(float) * 16);
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

		memcpy(data + location, matrix, sizeof(float) * 16);

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + sizeof(float) * 16; return messageSize; }
};

struct MaterialConnectionMessage : public Message
{
	size_t materialNameLength;
	char* materialName;

	MaterialConnectionMessage(size_t nameLength, char* name, size_t materialNameLength, char* materialName)
		:Message(NODETYPE::MATERIALCONNECTION, MESSAGETYPE::CHANGED, nameLength, name), materialNameLength(materialNameLength + 1), materialName(materialName) {}

	MaterialConnectionMessage(char* data)
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

		memcpy(&materialNameLength, data + location, sizeof(size_t));
		location += sizeof(size_t);

		materialName = new char[materialNameLength];
		strcpy_s(materialName, materialNameLength, data + location);
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

		memcpy(data + location, &materialNameLength, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(data + location, materialName, materialNameLength);

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 3 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + materialNameLength; return messageSize; }
};

struct MaterialChangedMessage : public Message
{
	size_t filePathLength;
	char* filePath;
	float color[3];

	MaterialChangedMessage(size_t nameLength, char* name, size_t filePathLength, char* filePath, float color[3])
		:Message(NODETYPE::MATERIAL, MESSAGETYPE::CHANGED, nameLength, name), filePathLength(filePathLength + 1), filePath(filePath)
	{
		memcpy(this->color, color, sizeof(float) * 3);
	}

	MaterialChangedMessage(char* data)
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

		memcpy(&filePathLength, data + location, sizeof(size_t));
		location += sizeof(size_t);

		if (filePathLength != 1)
		{
			filePath = new char[filePathLength];
			strcpy_s(filePath, filePathLength, data + location);
			location += filePathLength;
		}

		memcpy(color, data + location, sizeof(float) * 3);
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

		memcpy(data + location, &filePathLength, sizeof(size_t));
		location += sizeof(size_t);

		if (filePath)
		{
			memcpy(data + location, filePath, filePathLength);
			location += filePathLength;
		}

		memcpy(data + location, color, sizeof(float) * 3);

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 3 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + filePathLength + sizeof(float) * 3; return messageSize; }
};

struct NameChangedMessage : public Message
{
	size_t newNameLength;
	char* newName;

	NameChangedMessage(size_t nameLength, char* name, size_t newNameLength, char* newName)
		:Message(NODETYPE::NAMECHANGE, MESSAGETYPE::CHANGED, nameLength, name), newNameLength(newNameLength + 1), newName(newName) {}

	NameChangedMessage(char* data)
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

		memcpy(&newNameLength, data + location, sizeof(size_t));
		location += sizeof(size_t);

		newName = new char[newNameLength];
		strcpy_s(newName, newNameLength, data + location);
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

		memcpy(data + location, &newNameLength, sizeof(size_t));
		location += sizeof(size_t);

		memcpy(data + location, newName, newNameLength);

		return data;
	}

	virtual size_t Size() override { messageSize = sizeof(size_t) * 3 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + newNameLength; return messageSize; }
};