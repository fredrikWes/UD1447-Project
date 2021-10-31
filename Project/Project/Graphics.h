#pragma once
#include <d3d11.h>
#include <string>


enum class Shader { VS, PS };

class Graphics
{
private:
	static constexpr float backgroundColor[4] = { 0.1f, 0.2f, 0.5f, 1.0f };

	static D3D11_VIEWPORT viewport;
	static ID3D11Device* device;
	static ID3D11DeviceContext* context;
	static IDXGISwapChain* swapChain;
	static ID3D11RenderTargetView* backBuffer;
	static ID3D11Texture2D* dsTexture;
	static ID3D11DepthStencilView* dsView;
	static ID3D11SamplerState* sampler;

	static HRESULT CreateDeviceSwapchain(UINT clientWidth, UINT clientHeight, HWND hWnd);
	static HRESULT CreateRenderTarget();
	static HRESULT CreateDepthStencil(UINT clientWidth, UINT clientHeight);
	static HRESULT CreateSampler();
	static void CreateViewport(UINT clientWidth, UINT clientHeight);
public:
	static bool Initialize(UINT clientWidth, UINT clientHeight, HWND hWnd);
	static void ShutDown();

	static void SetViewPort(int width, int height);

	static void BeginFrame();
	static void EndFrame() { swapChain->Present(0, 0); }

	//BUFFERS
	static bool CreateConstantBuffer(ID3D11Buffer*& buffer, UINT size = 16);

	static bool CreateStructuredBuffer(ID3D11Buffer*& buffer, ID3D11ShaderResourceView*& srv, UINT stride, UINT maxElements);	//FOR LIGHTS LATER ON

	template <typename T>
	static bool UpdateConstantBuffer(ID3D11Buffer*& buffer, const T& data);

	static bool CreateVertexBuffer(ID3D11Buffer*& buffer, int stride, int numElements, const void* data);

	static void BindConstantBuffer(ID3D11Buffer* buffer, Shader shader = Shader::VS, UINT slot = 0);
	static void BindVertexBuffer(ID3D11Buffer* buffer, const UINT* stride, const UINT* offset);

	//TEXTURE2D
	static bool CreateTexture2D(ID3D11Texture2D*& texture, const D3D11_TEXTURE2D_DESC* desc, const D3D11_SUBRESOURCE_DATA* data);

	//SHADER RESOURCE
	static bool CreateShaderResourceView(ID3D11ShaderResourceView*& srv, const D3D11_SHADER_RESOURCE_VIEW_DESC* desc, ID3D11Resource* resource);
	static void BindShaderResourceView(ID3D11ShaderResourceView* srv, Shader shader = Shader::PS, UINT slot = 0);

	//INPUT LAYOUT
	static bool CreateInputLayout(ID3D11InputLayout*& inputLayout, const D3D11_INPUT_ELEMENT_DESC* desc, UINT numElements, std::string byteCode);

	static void BindInputLayout(ID3D11InputLayout* inputLayout);

	//SHADERS
	static bool CreateShader(ID3D11VertexShader*& shader, std::string path, std::string& byteCode);
	static bool CreateShader(ID3D11PixelShader*& shader, std::string path);

	static void BindShaders(ID3D11VertexShader* vertexShader, ID3D11PixelShader* shader);

	//MISC
	static void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
	static void Draw(UINT vertexCount);
};

inline D3D11_VIEWPORT Graphics::viewport;
inline ID3D11Device* Graphics::device;
inline ID3D11DeviceContext* Graphics::context;
inline IDXGISwapChain* Graphics::swapChain;
inline ID3D11RenderTargetView* Graphics::backBuffer;
inline ID3D11Texture2D* Graphics::dsTexture;
inline ID3D11DepthStencilView* Graphics::dsView;
inline ID3D11SamplerState* Graphics::sampler;

template<typename T>
inline bool Graphics::UpdateConstantBuffer(ID3D11Buffer*& buffer, const T& data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	if FAILED(context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))
		return false;

	memcpy(mappedResource.pData, &data, sizeof(data));
	context->Unmap(buffer, 0);
	return true;
}