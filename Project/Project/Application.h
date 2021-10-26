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

		Matrix viewMatrix = Matrix::CreateLookAt({ 10.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		Matrix perspectiveMatrix = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PIDIV4, (float)window.ClientWidth() / window.ClientHeight(), 0.1f, 100.0f);

		renderer->UpdateCameraMatrix((viewMatrix * perspectiveMatrix).Transpose());
	}

	~Application()
	{
		Graphics::ShutDown();
		window.ShutDown();
	}

	void Run()
	{
		while (!window.Exit())
		{
			MSG msg = {};
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (GetAsyncKeyState(VK_ESCAPE))
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

							if (nodes.find(message.name) != nodes.end())
								std::dynamic_pointer_cast<Mesh>(nodes[message.name])->Update(message);
						}

						if (msg->messageType == MESSAGETYPE::REMOVED)
						{
							renderer->Unbind(msg->name);
							nodes.erase(msg->name);
						}

						break;
					}

					case NODETYPE::TRANSFORM:
					{
						TransformChangedMessage message = TransformChangedMessage(data);

						if (nodes.find(message.name) != nodes.end())
							std::dynamic_pointer_cast<Mesh>(nodes[message.name])->matrix = Matrix(message.matrix);

						break;
					}

					case NODETYPE::CAMERA:
					{
						if (msg->messageType == MESSAGETYPE::CHANGED)
						{
							CameraChangedMessage message = CameraChangedMessage(data);

							Matrix viewMatrix, perspectiveMatrix;

							viewMatrix = Matrix(message.viewMatrix);
							perspectiveMatrix = Matrix(message.perspectiveMatrix);

							Matrix finalMatrix = Matrix((viewMatrix * perspectiveMatrix).Transpose());

							renderer->UpdateCameraMatrix(finalMatrix);							
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