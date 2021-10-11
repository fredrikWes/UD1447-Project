#pragma once
#include "FileMap.h"

struct CircularBuffer
{
	static const size_t headLocation = 0;
	static const size_t tailLocation = sizeof(size_t);
	static const size_t dataLocation = sizeof(size_t) * 3;
	size_t head = 0;
	size_t tail = 0;
	size_t size = 0;

	CircularBuffer() = default;
	CircularBuffer(void* data, const size_t size) : size(size - dataLocation) {}

	void SetHead(void* data) { memcpy((char*)data + headLocation, &head, sizeof(size_t)); }
	void SetTail(void* data) { memcpy((char*)data + tailLocation, &tail, sizeof(size_t)); }

	void GetHead(const void* data) { memcpy(&head, (char*)data + headLocation, sizeof(size_t)); }
	void GetTail(const void* data) { memcpy(&tail, (char*)data + tailLocation, sizeof(size_t)); }
};
