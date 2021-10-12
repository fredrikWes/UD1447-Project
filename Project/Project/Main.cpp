#include "SharedMemory.h"

int main()
{
	SharedMemory memory;

	char* test = new char[memory.MemorySize() / 2];
	size_t msgSize = 0;

	memory.Receive(test, msgSize);

	NodeAddedMessage msg(test);

	std::cout << msg.messageSize << " " << msg.nameLength << " " << (UINT)msg.type << " " << msg.name << std::endl;

	delete[] test;

	(void)getchar();
	return 0;
}