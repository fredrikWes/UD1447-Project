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

		switch (msg->type)
		{
		case NODETYPE::MESH:
		{
			NodeAddedMessage nodeMsg(data);
			std::cout << nodeMsg.messageSize << " " << nodeMsg.nameLength << " " << (UINT)nodeMsg.type << " " << nodeMsg.name << std::endl;
			break;
		}

		case NODETYPE::POINTLIGHT:
		{
			NodeAddedMessage nodeMsg(data);
			std::cout << nodeMsg.messageSize << " " << nodeMsg.nameLength << " " << (UINT)nodeMsg.type << " " << nodeMsg.name << std::endl;
			break;
		}
		}

		delete[] data;
	}

	(void)getchar();
	return 0;
}