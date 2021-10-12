#pragma once
enum class NODETYPE { MESH, POINTLIGHT, CAMERA };
enum class ATTRIBUTETYPE { GEOMETRY, TRANSFORM };

struct Message
{
	size_t messageSize;
	NODETYPE type;
};

struct NodeAddedMessage : public Message
{
	size_t nameLength;
	char* name;

	NodeAddedMessage(char* data)
	{
		//memcpy(&messageSize, data, sizeof(size_t));
		//memcpy(&type, data + sizeof(size_t), sizeof(NODETYPE));
		//memcpy(&nameLength, data + sizeof(size_t) + sizeof(NODETYPE), sizeof(size_t));
		name = new char[nameLength];
		strcpy_s(name, nameLength, data + sizeof(size_t) + sizeof(NODETYPE) + sizeof(size_t));
	}

	void Test(char* data)
	{
		name = new char[nameLength];
		strcpy_s(name, nameLength, data + sizeof(size_t) + sizeof(NODETYPE) + sizeof(size_t));
	}

	NodeAddedMessage(NODETYPE type, char* name, size_t nameLength) : nameLength(nameLength + 1) { this->messageSize = 0; this->name = name + '\0'; this->type = type; }
	size_t Size() { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + nameLength; return messageSize; }
};