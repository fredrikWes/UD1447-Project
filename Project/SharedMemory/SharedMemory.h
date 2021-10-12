#pragma once
#include "CircularBuffer.h"
#include "Messages.h"

class SharedMemory
{
private:
	FileMap fileMap;
	CircularBuffer buffer;
public:
	SharedMemory();
	bool Send(const void* data, const size_t size);
	bool Receive(void* data, size_t& size);
	size_t MemorySize() { return buffer.size; }
};