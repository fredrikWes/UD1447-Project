#include "SharedMemory.h"

SharedMemory::SharedMemory()
{
    buffer = CircularBuffer(fileMap.view, fileMap.size);
}

bool SharedMemory::Send(const void* data, const size_t size)
{
    buffer.GetTail(fileMap.view);

    if (buffer.head + size > buffer.size)
        buffer.head = 0;

    if (buffer.head < buffer.tail && buffer.head + size > buffer.tail)
        return false;

    memcpy((char*)fileMap.view + CircularBuffer::dataLocation + buffer.head, data, size);

    buffer.head += size;
    buffer.SetHead(fileMap.view);

    return true;
}

bool SharedMemory::Receive(void* data, size_t& size)
{
    buffer.GetHead(fileMap.view);

    if (buffer.tail + sizeof(UINT) > buffer.size)
        buffer.tail = 0;

    if (buffer.tail < buffer.head && buffer.tail + sizeof(UINT) > buffer.head)    //Check if we can read first UINT (message size)
        return false;

    size_t messageSize = 0;
    memcpy(&messageSize, (char*)fileMap.view + CircularBuffer::dataLocation + buffer.tail, sizeof(UINT));
    size = messageSize;

    if (buffer.tail + size > buffer.size)
        buffer.tail = 0;

    if (buffer.tail < buffer.head && buffer.tail + size > buffer.head)    //Check if we can read whole message
        return false;

    memcpy(data, (char*)fileMap.view + CircularBuffer::dataLocation + buffer.tail, size);

    buffer.tail += size;
    buffer.SetTail(fileMap.view);

    return true;
}