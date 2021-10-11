#pragma once
#include "CircularBuffer.h"

enum class NODETYPE { MESH, POINTLIGHT, CAMERA };
enum class ATTRIBUTETYPE { GEOMETRY, TRANSFORM };

class SharedMemory
{
private:
	FileMap fileMap;
	CircularBuffer buffer;
public:
	SharedMemory();
	bool Send(const void* data, const size_t size);
	bool Receive(void* data, size_t& size);
};