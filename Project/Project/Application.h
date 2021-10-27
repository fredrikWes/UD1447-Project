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

		Matrix viewMatrix = Matrix::CreateLookAt({ 10.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		Matrix perspectiveMatrix = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PIDIV4, (float)window.ClientWidth() / window.ClientHeight(), 0.1f, 100.0f);

		renderer = std::make_unique<Renderer>();

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

			while (true)
			{
				bool recieved = memory.Receive(data, msgSize);
				if (!recieved)
					break;

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
					std::cout << "TRANSFORM MESSAGE RECIEVED, SIZE: " << message.messageSize << std::endl;

					if (nodes.find(message.name) != nodes.end())
						std::dynamic_pointer_cast<Mesh>(nodes[message.name])->matrix = Matrix(message.matrix);

					break;
				}

				case NODETYPE::CAMERA:
				{
					if (msg->messageType == MESSAGETYPE::CHANGED)
					{
						CameraChangedMessage message = CameraChangedMessage(data);
						std::cout << "CAMERA MESSAGE RECIEVED, SIZE: " << message.messageSize << std::endl;

						Matrix viewMatrix, perspectiveMatrix;

						/*std::cout << "Name: " << message.name << std::endl;
						std::cout << "Messagelenth: " << message.messageSize << std::endl;
						std::cout << "MessageType: " << (UINT)message.messageType << std::endl;
						std::cout << "OrthoWidth: " << message.orthoWidth << std::endl;
						std::cout << "Orthographic: " << message.orthographic << std::endl;
						std::cout << "NearZ: " << message.nearZ << std::endl;
						std::cout << "FarZ: " << message.farZ << std::endl;
						std::cout << "Vertical FOV: " << message.verFOV << std::endl;
						std::cout << "Horizontal FOV: " << message.horFOV << std::endl;
						std::cout << "EyePos: " << message.eyePos[0] << std::endl;
						std::cout << "EyePos: " << message.eyePos[1] << std::endl;
						std::cout << "EyePos: " << message.eyePos[2] << std::endl;
						std::cout << "EyePos: " << message.eyePos[3] << std::endl;
						std::cout << "Center: " << message.center[0] << std::endl;
						std::cout << "Center: " << message.center[1] << std::endl;
						std::cout << "Center: " << message.center[2] << std::endl;
						std::cout << "Center: " << message.center[3] << std::endl;
						std::cout << "Up: " << message.up[0] << std::endl;
						std::cout << "Up: " << message.up[1] << std::endl;
						std::cout << "Up: " << message.up[2] << std::endl;
						std::cout << "PortWidth: " << message.portWidth << std::endl;
						std::cout << "PortHeight: " << message.portHeight << std::endl;*/

						viewMatrix = DirectX::XMMatrixLookAtLH(Vector4(message.eyePos[2], message.eyePos[1], message.eyePos[0], message.eyePos[3]),
							Vector4(message.center[2], message.center[1], message.center[0], message.center[3]),
							Vector3((float)message.up[2], (float)message.up[1], (float)message.up[0]));

						float aspectRatio = (float)message.portWidth / message.portHeight;
						float orthoAspectRatio = (float)message.portHeight / message.portWidth;

						Graphics::SetViewPort(message.portWidth, message.portHeight);

						if (message.orthographic)
							perspectiveMatrix = DirectX::XMMatrixOrthographicLH(message.orthoWidth, message.orthoWidth * orthoAspectRatio, message.nearZ, message.farZ);

						else
							perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, message.nearZ, message.farZ);

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