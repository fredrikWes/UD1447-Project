#include "SharedMemory.h"

int main()
{
	SharedMemory memory;

	char* test = new char[memory.MemorySize() / 2];
	size_t msgSize = 0;

	memory.Receive(test, msgSize);

	NodeAddedMessage* msg = (NodeAddedMessage*)test;
	msg->Test(test);

	auto len = strlen(msg->name);

	std::cout << msg->messageSize << " " << msg->nameLength << " " << (UINT)msg->type << std::endl;

	delete[] test;

	(void)getchar();
	return 0;
}