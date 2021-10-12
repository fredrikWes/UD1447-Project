#include "SharedMemory.h"

SharedMemory::SharedMemory()
{
    buffer = CircularBuffer(fileMap.view, fileMap.size);
    if (!fileMap.exists)
        memset((char*)fileMap.view + CircularBuffer::dataLocation, -1, buffer.size);
}

bool SharedMemory::Send(const void* data, const size_t size)
{
    buffer.GetTail(fileMap.view);

    if (buffer.head + size > buffer.size)
    {
        if (buffer.head < buffer.tail)
            return false;

        if (buffer.head + sizeof(int) < buffer.size)
        {
            int reset = -1;
            memcpy((char*)fileMap.view + CircularBuffer::dataLocation + buffer.head, &reset, sizeof(int));
        }

        buffer.head = 0;
    }

    if (buffer.head == buffer.tail && buffer.head == 0)
    {
        if ((int)*((char*)fileMap.view + CircularBuffer::dataLocation + buffer.head) != -1)
            return false;
    }

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

    if (buffer.tail + sizeof(int) > buffer.size)
    {
        if (buffer.tail <= buffer.head)
            return false;

        buffer.tail = 0;
    }
       
    if (buffer.tail <= buffer.head && buffer.tail + sizeof(int) > buffer.head)    //Check if we can read first UINT (message size)
        return false;

    int messageSize = 0;
    memcpy(&messageSize, (char*)fileMap.view + CircularBuffer::dataLocation + buffer.tail, sizeof(int));
    size = messageSize;

    if (messageSize == -1)
    {
        buffer.tail = 0;
        return false;
    }
        
    if (buffer.tail <= buffer.head && buffer.tail + size > buffer.head)    //Check if we can read whole message
        return false;

    memcpy(data, (char*)fileMap.view + CircularBuffer::dataLocation + buffer.tail, size);
    memset((char*)fileMap.view + CircularBuffer::dataLocation + buffer.tail, -1, size);

    buffer.tail += size;
    buffer.SetTail(fileMap.view);

    std::cout << "================================" << std::endl;
    std::cout << "HEAD: " << buffer.head << " " << (int)*((char*)fileMap.view + CircularBuffer::dataLocation + buffer.head) << std::endl;
    std::cout << "TAIL: " << buffer.tail << " " << (int)*((char*)fileMap.view + CircularBuffer::dataLocation + buffer.tail) << std::endl;
    std::cout << "SIZE: " << buffer.size << " " << std::endl;

    return true;
}