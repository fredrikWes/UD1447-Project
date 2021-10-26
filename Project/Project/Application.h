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
							for (auto& [name, node] : nodes)
							{
								if (std::string(message.name).find(name))
								{
									std::dynamic_pointer_cast<Mesh>(nodes[name])->Update(message);
									break;
								}
							}
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

						for (auto& [name, node] : nodes)
						{
							if (std::string(message.name).find(name))
							{
								std::dynamic_pointer_cast<Mesh>(nodes[message.name])->matrix = Matrix(message.matrix);
								break;
							}
						}

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
							
							/*viewMatrix = Matrix(message.viewMatrix[0], message.viewMatrix[1], message.viewMatrix[2], message.viewMatrix[3], message.viewMatrix[4],
								message.viewMatrix[5], message.viewMatrix[6], message.viewMatrix[7], message.viewMatrix[8], message.viewMatrix[9],
								message.viewMatrix[10], message.viewMatrix[11], message.viewMatrix[12], message.viewMatrix[13],
								message.viewMatrix[14], message.viewMatrix[15]);*/
						
							/*perspectiveMatrix = Matrix(message.perspectiveMatrix[0], message.perspectiveMatrix[1], message.perspectiveMatrix[2], message.perspectiveMatrix[3], message.perspectiveMatrix[4],
								message.perspectiveMatrix[5], message.perspectiveMatrix[6], message.perspectiveMatrix[7], message.perspectiveMatrix[8], message.perspectiveMatrix[9],
								message.perspectiveMatrix[10], message.perspectiveMatrix[11], message.perspectiveMatrix[12], message.perspectiveMatrix[13],
								message.perspectiveMatrix[14], message.perspectiveMatrix[15]);*/

							if (message.orthographic)
							{
								//perspectiveMatrix = Matrix::CreateOrthographic(message.orthoWidth, message.orthoWidth, 0.1f, 100.0f);
							}

							else
							{
								//perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, (float)window.ClientWidth() / window.ClientHeight(), 0.1f, 100.0f);
								//perspectiveMatrix = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PIDIV4, (float)window.ClientWidth() / window.ClientHeight(), 0.1f, 100.0f);
							}
							
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