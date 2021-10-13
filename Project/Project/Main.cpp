#include "SharedMemory.h"

int main()
{
	SharedMemory memory;
	bool exit = false;

	while (!exit)
	{
		if (GetAsyncKeyState(VK_RETURN))
			exit = true;

		char* data = new char[memory.MemorySize() / 2];
		size_t msgSize = 0;

		bool received = false;
		while (!received)
			received = memory.Receive(data, msgSize);

		Message* msg = new Message(data);

		std::cout << msg->messageSize << " " << msg->nameLength << " " << (UINT)msg->nodeType << " " << (UINT)msg->messageType << " " << msg->name << std::endl;

		if (msg)
			delete msg;

		if (data)
			delete[] data;
	}

	(void)getchar();
	return 0;
}