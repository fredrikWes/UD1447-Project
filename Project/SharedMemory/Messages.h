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

		/*for (UINT i = 0; i < 16; ++i)
		{
			this->viewMatrix[i] = vMatrix[i];
			this->perspectiveMatrix[i] = pMatrix[i];
		}*/
			
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

		/*memcpy(viewMatrix, data + location, sizeof(float) * 16);
		location += sizeof(float) * 16;

		memcpy(perspectiveMatrix, data + location, sizeof(float) * 16);
		location += sizeof(float) * 16;*/

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

		/*memcpy(data + location, viewMatrix, sizeof(float) * 16);
		location += sizeof(float) * 16;

		memcpy(data + location, perspectiveMatrix, sizeof(float) * 16);
		location += sizeof(float) * 16;*/

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

	virtual size_t Size() override { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + /*sizeof(float) * 16 +  sizeof(float) * 16 +*/ sizeof(double) + sizeof(bool) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) * 4 + sizeof(float) * 4 + sizeof(double) * 3 + sizeof(int) + sizeof(int); return messageSize; }
};

struct MeshChangedMessage : public Message
{
	int vertexCount;

	int numIndices;
	int* indices;

	int numVertices;
	Vertex* vertices;

	MeshChangedMessage(NODETYPE nodeType, MESSAGETYPE messageType, size_t nameLength, char* name, int* indices, int numIndices, Vertex* vertices, int numVertices, int vertexCount)
		:Message(nodeType, messageType, nameLength, name), numIndices(numIndices), numVertices(numVertices), vertexCount(vertexCount)
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

		memcpy(&vertexCount, data + location, sizeof(int));
		location += sizeof(int);

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

		memcpy(data + location, &vertexCount, sizeof(int));
		location += sizeof(int);

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

	virtual size_t Size() override { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + sizeof(MESSAGETYPE) + nameLength + sizeof(int) * 3 + sizeof(int) * numIndices + sizeof(Vertex) * numVertices; return messageSize; }
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