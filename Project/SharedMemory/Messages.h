#pragma once
enum class NODETYPE { MESH, POINTLIGHT, CAMERA };
enum class ATTRIBUTETYPE { GEOMETRY, TRANSFORM };

struct Message
{
	size_t messageSize;
	NODETYPE type;

	Message() = default;
	Message(char* data)
	{
		memcpy(&messageSize, data, sizeof(size_t));
		memcpy(&type, data + sizeof(size_t), sizeof(NODETYPE));
	}
};

struct NodeAddedMessage : public Message
{
	size_t nameLength;
	char* name;

	NodeAddedMessage(char* data)
		:nameLength(0)
	{
		memcpy(&messageSize, data, sizeof(size_t));
		memcpy(&type, data + sizeof(size_t), sizeof(NODETYPE));
		memcpy(&nameLength, data + sizeof(size_t) + sizeof(NODETYPE), sizeof(size_t));
		name = new char[nameLength];
		strcpy_s(name, nameLength, data + sizeof(size_t) + sizeof(NODETYPE) + sizeof(size_t));
	}

	NodeAddedMessage(NODETYPE type, char* name, size_t nameLength) : nameLength(nameLength + 1) { this->messageSize = 0; this->name = name; this->type = type; }

	void* Data() 
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

	size_t Size() { messageSize = sizeof(size_t) * 2 + sizeof(NODETYPE) + nameLength; return messageSize; }
};