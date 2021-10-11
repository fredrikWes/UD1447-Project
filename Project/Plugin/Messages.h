#pragma once
#include "SharedMemory.h"
#include "MayaIncludes.h"

struct NodeAddedMessage
{
	size_t messageSize;
	NODETYPE type;
	size_t nameLength;
	const char* name;

	NodeAddedMessage(NODETYPE type, const char* name, size_t nameLength): type(type), nameLength(nameLength) { this->name = name; }
	size_t Size() { return sizeof(size_t) * 2 + sizeof(NODETYPE) + nameLength; }
};