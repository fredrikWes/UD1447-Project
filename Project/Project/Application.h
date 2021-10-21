#pragma once
#include "Window.h"
#include "Renderer.h"

class Application
{
private:
	Window window;
	SharedMemory memory;
	std::unique_ptr<Renderer> renderer;
	std::map<std::string, std::shared_ptr<Node>> nodes;
public:
	Application(HINSTANCE instance)
	{
		const UINT WIDTH = 1280;
		const UINT HEIGHT = 760;

		WindowCreator creator;
		creator.Initialize(window, WIDTH, HEIGHT, L"Window", instance);

		Graphics::Initialize(window.ClientWidth(), window.ClientHeight(), window.GetHWND());

		renderer = std::make_unique<Renderer>();
	}

	~Application()
	{
		Graphics::ShutDown();
		window.ShutDown();
	}

	void Run()
	{
		Matrix viewMatrix = Matrix::CreateLookAt({ -10.0f, 0.0f, -10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		Matrix perspectiveMatrix = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PIDIV4, (float)window.ClientWidth() / window.ClientHeight(), 0.1f, 100.0f);

		renderer->UpdateCameraMatrix((viewMatrix * perspectiveMatrix).Transpose());

		while (!window.Exit())
		{
			MSG msg = {};
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (GetAsyncKeyState(VK_RETURN))
				break;

			//RENDERING	
			Graphics::BeginFrame();

			renderer->Render();

			Graphics::EndFrame();

			//MESSAGES
			char* data = new char[memory.MemorySize() / 2];
			size_t msgSize = 0;

			bool received = memory.Receive(data, msgSize);

			if (received)
			{
				Message* msg = new Message(data);

				switch (msg->nodeType)
				{
					case NODETYPE::MESH:
					{
						if (msg->messageType == MESSAGETYPE::ADDED)
						{
							auto mesh = std::make_shared<Mesh>(*msg);
							nodes[mesh->name] = mesh;
							renderer->Bind(mesh->name, mesh);
						}

						if (msg->messageType == MESSAGETYPE::CHANGED)
						{
							MeshChangedMessage message = MeshChangedMessage(data);
							auto mesh = std::dynamic_pointer_cast<Mesh>(nodes[message.name]);
							mesh->Update(message);
						}

						if (msg->messageType == MESSAGETYPE::REMOVED)
						{
							renderer->Unbind(msg->name);
							nodes.erase(msg->name);
						}

						break;
					}
					case NODETYPE::CAMERA:
					{

						if (msg->messageType == MESSAGETYPE::CHANGED)
						{
							CameraChangedMessage message = CameraChangedMessage(data);
							std::cout << "Name: " << message.name << std::endl;
							std::cout << "Messagelenth: " << message.messageSize << std::endl;
							std::cout << "MessageType: " << (UINT)message.messageType << std::endl;
							std::cout << "OrthoWidth: " << message.orthoWidth << std::endl;
							for (UINT i = 0; i < 16; i++)
							{
								std::cout << "Matrix: " << message.matrix[i] << std::endl;
							}
							

							//renderer->UpdateCameraMatrix();
						}
						break;
					}

				}

				if (msg)
					delete msg;
			}

			if (data)
				delete[] data;
		}
	}
};