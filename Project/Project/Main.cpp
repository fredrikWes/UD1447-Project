#include "DataTypes.h"

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

		switch (msg->nodeType)
		{
			case NODETYPE::MESH:
			{
				if (msg->messageType == MESSAGETYPE::CHANGED)
				{
					MeshChangedMessage message = MeshChangedMessage(data);
					std::cout << message.messageSize << " " << (UINT)message.nodeType << " " << (UINT)message.messageType << " " << message.nameLength << " " << message.name << std::endl;
					std::cout << message.numVertices << std::endl;
					for (UINT i = 0; i < message.numVertices; ++i)
						std::cout << message.vertices[i].Px << " " << message.vertices[i].Py << " " << message.vertices[i].Pz << std::endl;
					break;
				}
				break;
			}
		}
		
		if (msg)
			delete msg;

		if (data)
			delete[] data;
	}

	(void)getchar();
	return 0;
}