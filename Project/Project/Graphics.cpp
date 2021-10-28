#include "Graphics.h"

#include <iostream>
#include <fstream>

HRESULT Graphics::CreateDeviceSwapchain(UINT clientWidth, UINT clientHeight, HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = clientWidth;
	swapChainDesc.BufferDesc.Height = clientHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = true;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	UINT flags = 0;

#ifdef _DEBUG
	flags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	return D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &context);
}

HRESULT Graphics::CreateRenderTarget()
{
	HRESULT hr;
	ID3D11Texture2D* buffer = nullptr;

	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&buffer));

	if FAILED(hr)
		return hr;

	hr = device->CreateRenderTargetView(buffer, nullptr, &backBuffer);

	buffer->Release();

	return hr;
}

HRESULT Graphics::CreateDepthStencil(UINT clientWidth, UINT clientHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};

	textureDesc.Width = clientWidth;
	textureDesc.Height = clientHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &dsTexture);
	if FAILED(hr)
		return hr;

	return device->CreateDepthStencilView(dsTexture, nullptr, &dsView);
}

void Graphics::CreateViewport(UINT clientWidth, UINT clientHeight)
{
	viewport.Width = static_cast<float>(clientWidth);
	viewport.Height = static_cast<float>(clientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
}

HRESULT Graphics::CreateSampler()
{
	D3D11_SAMPLER_DESC desc = {};

	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	
	return device->CreateSamplerState(&desc, &sampler);
}

bool Graphics::Initialize(UINT clientWidth, UINT clientHeight, HWND hWnd)
{
	if FAILED(CreateDeviceSwapchain(clientWidth, clientHeight, hWnd))
		return false;

	if FAILED(CreateRenderTarget())
		return false;

	if FAILED(CreateDepthStencil(clientWidth, clientHeight))
		return false;

	if FAILED(CreateSampler())
		return false;

	CreateViewport(clientWidth, clientHeight);

	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, &backBuffer, dsView);
	context->PSSetSamplers(0, 1, &sampler);

	std::cout << ">>> SUCCEDDED TO INITIALIZE GRAPHICS <<<" << std::endl;
	return true;
}

void Graphics::ShutDown()
{
	sampler->Release();
	dsView->Release();
	dsTexture->Release();
	backBuffer->Release();
	swapChain->Release();
	context->Release();
	device->Release();
}
#include "Window.h"
void Graphics::SetViewPort(int width, int height)
{
	
	SetWindowPos(Window::GetHWND(), NULL, 0, 0, width, height, SWP_SHOWWINDOW | SWP_NOMOVE);

	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
}

void Graphics::BeginFrame()
{
	context->ClearRenderTargetView(backBuffer, backgroundColor);
	context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

bool Graphics::CreateConstantBuffer(ID3D11Buffer*& buffer, UINT size)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.ByteWidth = size;

	if FAILED(device->CreateBuffer(&bufferDesc, nullptr, &buffer))
		return false;
	return true;
}

bool Graphics::CreateVertexBuffer(ID3D11Buffer*& buffer, int stride, int numElements, const void* data)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = stride * numElements;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = stride;

	D3D11_SUBRESOURCE_DATA resourceData = {};
	resourceData.pSysMem = data;

	if FAILED(device->CreateBuffer(&desc, &resourceData, &buffer))
		return false;
	return true;
}

void Graphics::BindConstantBuffer(ID3D11Buffer* buffer, Shader shader, UINT slot)
{
	switch (shader)
	{
	case Shader::VS:
		context->VSSetConstantBuffers(slot, 1, &buffer);
		break;

	case Shader::PS:
		context->PSSetConstantBuffers(slot, 1, &buffer);
		break;
	}
}

void Graphics::BindVertexBuffer(ID3D11Buffer* buffer, const UINT* stride, const UINT* offset)
{
	context->IASetVertexBuffers(0, 1, &buffer, stride, offset);
}

bool Graphics::CreateTexture2D(ID3D11Texture2D*& texture, const D3D11_TEXTURE2D_DESC* desc, const D3D11_SUBRESOURCE_DATA* data)
{
	if FAILED(device->CreateTexture2D(desc, data, &texture))
		return false;
	return true;
}

bool Graphics::CreateShaderResourceView(ID3D11ShaderResourceView*& srv, const D3D11_SHADER_RESOURCE_VIEW_DESC* desc, ID3D11Resource* resource)
{
	if FAILED(device->CreateShaderResourceView(resource, desc, &srv))
		return false;
	return true;
}

void Graphics::BindShaderResourceView(ID3D11ShaderResourceView* srv, Shader shader, UINT slot)
{
	switch (shader)
	{
	case Shader::VS:
		context->VSSetShaderResources(slot, 1, &srv);
		break;

	case Shader::PS:
		context->PSSetShaderResources(slot, 1, &srv);
		break;
	}
}

bool Graphics::CreateInputLayout(ID3D11InputLayout*& inputLayout, const D3D11_INPUT_ELEMENT_DESC* desc, UINT numElements, std::string byteCode)
{
	if FAILED(device->CreateInputLayout(desc, numElements, byteCode.c_str(), byteCode.length(), &inputLayout))
		return false;
	return true;
}

void Graphics::BindInputLayout(ID3D11InputLayout* inputLayout)
{
	context->IASetInputLayout(inputLayout);
}

bool Graphics::CreateShader(ID3D11VertexShader*& shader, std::string path, std::string& byteCode)
{
	std::string shaderData;
	std::ifstream reader;

	reader.open(path, std::ios::binary | std::ios::beg);

	if (!reader.is_open())
		return false;

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);
	shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());

	HRESULT hr = device->CreateVertexShader(shaderData.c_str(), shaderData.length(), nullptr, &shader);
	if FAILED(hr)
		return false;

	byteCode = shaderData;
	shaderData.clear();
	reader.close();

	return true;
}

bool Graphics::CreateShader(ID3D11PixelShader*& shader, std::string path)
{
	std::string shaderData;
	std::ifstream reader;

	reader.open(path, std::ios::binary | std::ios::beg);

	if (!reader.is_open())
		return false;

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);
	shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());

	HRESULT hr = device->CreatePixelShader(shaderData.c_str(), shaderData.length(), nullptr, &shader);
	if FAILED(hr)
		return false;

	shaderData.clear();
	reader.close();

	return true;
}

void Graphics::BindShaders(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader)
{
	context->VSSetShader(vertexShader, nullptr, 0);
	context->PSSetShader(pixelShader, nullptr, 0);
}

void Graphics::SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	context->IASetPrimitiveTopology(topology);
}

void Graphics::Draw(UINT vertexCount)
{
	context->Draw(vertexCount, 0);
}