#include "D3D10Renderer.h"

#include <D3D10.h>
#include <D3DX10.h>

D3D10Renderer::D3D10Renderer()
{
	m_pD3D10Device = NULL;
	m_pSwapChain = NULL;
	m_pRenderTargetView = NULL;
	m_pDepthStencelView = NULL;
	m_pDepthStencilTexture = NULL;
}

D3D10Renderer::~D3D10Renderer()
{
	if (m_pD3D10Device)
		m_pD3D10Device->ClearState();

	if(m_pSwapChain)
		m_pSwapChain->Release();
	if(m_pRenderTargetView)
		m_pRenderTargetView->Release();
	if(m_pDepthStencelView)
		m_pDepthStencelView->Release();
	if(m_pDepthStencilTexture)
		m_pDepthStencilTexture->Release();
	if (m_pD3D10Device)
		m_pD3D10Device->Release();
}

//Calculate the size of the window to be drawn
bool D3D10Renderer::init(void *pWindowHandle, bool fullScreen)
{
	HWND window = (HWND)pWindowHandle;
	RECT windowRect;
	GetClientRect(window, &windowRect);

	UINT width = windowRect.right - windowRect.left;
	UINT height = windowRect.bottom - windowRect.top;

	if(!createDevice(window, width, height, fullScreen))
		return false;
	if(!createInitialRenderTarget(width, height))
		return false;

	return true;
}

//Creates the device to interface with the graphics hardware and also a swapchain 
//which holds a series of drawing buffers
bool D3D10Renderer::createDevice(HWND window, int windowWidth, int windowHeight, bool fullScreen)
{

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	if(fullScreen)
		sd.BufferCount = 2;
	else
		sd.BufferCount = 1;
	sd.OutputWindow = window;
	sd.Windowed = (BOOL)(!fullScreen);
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferDesc.Width = windowWidth;
	sd.BufferDesc.Height = windowHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator =1;

	if(FAILED(D3D10CreateDeviceAndSwapChain(NULL, 
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		D3D10_SDK_VERSION,
		&sd,
		&m_pSwapChain,
		&m_pD3D10Device)))
		return false;

	return true;
}