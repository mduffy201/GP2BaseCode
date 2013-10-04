#include "D3D10Renderer.h"

//Include header files from DX10 library
#include <D3D10.h>
#include <D3DX10.h>

struct Vertex {
	float x, y, z;
};

const D3D10_INPUT_ELEMENT_DESC VertexLayout[] = 
{
	{"POSITION",
	0,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	0,
	0,
	D3D10_INPUT_PER_VERTEX_DATA,
	0},
};

const char basicEffect[]=\
	"float4 VS(float4 Pos:POSITION):SV_POSITION"\
	"{"\
	"	return Pos;"\
	"}"\
	"float4 PS(float4 Pos:SV_POSITION):SV_Target"\
	"{"\
	"return float4 (1.0f, 1.0f, 0.0f, 1.0f);"\
	"}"\
	"technique10 Render"\
	"{"\
	"	Pass P0"\
	"	{"\
	"		SetVertexShader(CompileShader(vs_4_0, VS()));"\
	"		SetGeometryShader(NULL);"\
	"		SetPixelShader(CompileShader(ps_4_0, PS()));"\
	"	}"\
	"}";
	

//Constructor
D3D10Renderer::D3D10Renderer()
{
	//Set all member variables to NULL
	m_pD3D10Device = NULL;
	m_pRenderTargetView = NULL;
	m_pSwapChain = NULL;
	
	m_pDepthStencelView = NULL;
	m_pDepthStencilTexture = NULL;

	m_pTempEffect = NULL;
	m_pTempTechnique = NULL;
	m_pTempBuffer = NULL;
	m_pTempVertexLayout = NULL;
}
//Destructor
D3D10Renderer::~D3D10Renderer()
{

	
	if (m_pD3D10Device)
		m_pD3D10Device->ClearState();

	if(m_pTempBuffer)
		m_pTempBuffer->Release();
	if(m_pTempEffect)
		m_pTempEffect->Release();
	if(m_pTempVertexLayout)
		m_pTempVertexLayout->Release();
	//Release each of the DX10 interfaces
	//Release(): Release the pointer when no referenced objects remain
	//From IUnkown Interface
	if(m_pRenderTargetView)
		m_pRenderTargetView->Release();
	if(m_pDepthStencelView)
		m_pDepthStencelView->Release();
	if(m_pDepthStencilTexture)
		m_pDepthStencilTexture->Release();
		if(m_pSwapChain)
		m_pSwapChain->Release();
	if (m_pD3D10Device)
		m_pD3D10Device->Release();
}
//Calculate the size of the window to be drawn
bool D3D10Renderer::init(void *pWindowHandle, bool fullScreen)
{
	HWND window = (HWND)pWindowHandle;
	RECT windowRect;
	//Returns the size of the clients area in windowRect
	GetClientRect(window, &windowRect);

	UINT width = windowRect.right - windowRect.left;
	UINT height = windowRect.bottom - windowRect.top;

	//Calls functions in D3D10Renderer.
	if(!createDevice(window, width, height, fullScreen))
		return false;
	if(!createInitialRenderTarget(width, height))
		return false;
/*if(!loadEffectFromMemory())
		return false;
	if(!createBuffer())
		return false;
	if(!createVertexLayout())
		return false;*/

	return true;
}
//Creates the device to interface with the graphics hardware and also a swapchain 
//which holds a series of drawing buffers
bool D3D10Renderer::createDevice(HWND window, int windowWidth, int windowHeight, bool fullScreen)
{

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags|=D3D10_CREATE_DEVICE_DEBUG;
#endif

	//DXGI_SWAP_CHAIN_DESC 
	//A struct which describes a swap chain
	//       variables
	//DXGI_MODE_DESC   BufferDesc - Describes the backbuffer display mode  
	//DXGI_SAMPLE_DESC SampleDesc - Describes multi-sampling parameters
	//DXGI_USAGE       BufferUsage - Describes the surface usage and CPU access options for the back buffer
	//UINT             BufferCount - Number of buffers in swap chain.
	//HWND             OutputWindow - The output window
	//BOOL             Windowed - Windowed mode
	//DXGI_SWAP_EFFECT SwapEffect - Describes options for handling the contents of the presentation buffer after presenting a surface
	//UINT             Flags - describes options for swap-chain behavior

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

	//D3D10CreateDeviceAndSwapChain
	//Create a Direct3D 10.0 device and a swap chain.

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
//Grabs the backbuffer from the swap chain and then creates
//a depth stencil texture
bool D3D10Renderer::createInitialRenderTarget(int windowWidth, int windowHeight)
{
	//ID3D10Texture2D 
	//Interface for a 2d texture used as a buffer
	//A pointer is declared which points to the location of this


	ID3D10Texture2D *pBackBuffer;

	//GetBuffer
	//Function to access one of the swapchains back buffers
	//[in]       UINT Buffer - The buffer index
	//[in]       REFIID riid - Type of interface used to manipulate the buffer
	//[in, out]  void **ppSurface - pointer to back buffer interface


	if (FAILED(m_pSwapChain->GetBuffer(0, 
		__uuidof(ID3D10Texture2D),
		(void**)&pBackBuffer))) 
		return false;


	//D3D10_TEXTURE2D_DESC
	//Struct which describes a 2d texture


	D3D10_TEXTURE2D_DESC descDepth;
	descDepth.Width=windowWidth;
	descDepth.Height=windowHeight;
	descDepth.MipLevels=1;
	descDepth.ArraySize=1;
	descDepth.Format=DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count=1;
	descDepth.SampleDesc.Quality=0;
	descDepth.Usage=D3D10_USAGE_DEFAULT;
	descDepth.BindFlags=D3D10_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags=0;
	descDepth.MiscFlags=0;


	//CreateTexture2D
	//Method to create an array of 2d textures
	//[in]   const D3D10_TEXTURE2D_DESC *pDesc - pointer to 2d texture description
	//[in]   const D3D10_SUBRESOURCE_DATA *pInitialData - pointer to array of subresource descriptions
	//[out]  ID3D10Texture2D **ppTexture2D - address of pointer to created texture


	if (FAILED(m_pD3D10Device->CreateTexture2D(&descDepth,NULL,
			&m_pDepthStencilTexture)))
		return false;

	//creates the DepthStencilView and the RenderTargetView, binds them both to the
	//pipeline and then finally creates a viewport; which is used in the pipeline for some of the transformations
	
	
	//D3D10_DEPTH_STENCIL_VIEW_DESC ===============
	//struct which specifies the subresource(s) from a texture that are accessible using a depth-stencil view.

	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format=descDepth.Format;
	descDSV.ViewDimension=D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice=0;

	//CreateDepthStencilView =================
	//Method which creates a depth-stencil view for accessing resource data.

	if (FAILED(m_pD3D10Device->CreateDepthStencilView(m_pDepthStencilTexture,
                   &descDSV,&m_pDepthStencelView)))
		return false;

	if (FAILED(m_pD3D10Device->CreateRenderTargetView( pBackBuffer, 
		NULL, 
		&m_pRenderTargetView ))){
             pBackBuffer->Release();
		return  false;
	}
       pBackBuffer->Release();
	   
	   //OMSetRenderTargets ==============
	   //Method to bind one or more render targets and the depth-stencil buffer to the output-merger stage.
	    //[in]  UINT NumViews - Number of render targets to bind.
  //[in]  ID3D10RenderTargetView *const *ppRenderTargetViews - Pointer to an array of render targets.
  //[in]  ID3D10DepthStencilView *pDepthStencilView - Pointer to a depth-stencil view.

	m_pD3D10Device->OMSetRenderTargets(1, 
		&m_pRenderTargetView,		
		m_pDepthStencelView);

	//D3D10_VIEWPORT================
	//Struct which defines the dimensions of a viewport

	D3D10_VIEWPORT vp;
   	vp.Width = windowWidth;
    vp.Height = windowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    
	//RSSetViewports===============
	//Method which binds an array of viewports to the rasterizer stage of the pipeline.

	m_pD3D10Device->RSSetViewports( 1 
		, &vp );
	return true;
}
//clears all buffers and then presents the swapchain which flip the back and the front buffers and low
//and behold an image will appear
void D3D10Renderer::clear(float r,float g,float b,float a)
{
    // Just clear the backbuffer, colours start at 0.0 to 1.0
	// Red, Green , Blue, Alpha - BMD
    const float ClearColor[4] = { r, g, b, a}; 
	//Clear the Render Target
	//http://msdn.microsoft.com/en-us/library/bb173539%28v=vs.85%29.aspx - BMD
    
	//ClearRenderTargetView=================
	//Method which sets all the elements in a render target to one value.

	m_pD3D10Device->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	
	//ClearDepthStencilView==============
	//Method to clear the depth-stencil resource

	m_pD3D10Device->ClearDepthStencilView(m_pDepthStencelView,D3D10_CLEAR_DEPTH,1.0f,0);
}
void D3D10Renderer::present()
{

	//Present================
	//Presents a rendered image to the user.

	//Swaps the buffers in the chain, the back buffer to the front(screen)
	//http://msdn.microsoft.com/en-us/library/bb174576%28v=vs.85%29.aspx - BMD
    m_pSwapChain->Present( 0, 0 );
}
void D3D10Renderer::render()
{}

bool D3D10Renderer::loadEffectFromMemory(const char* pMem){

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(DEBUG)||defined(_DEBUG)
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif
	
	ID3D10Blob * pErrorBuffer = NULL;

	if (FAILED(D3DX10CreateEffectFromMemory(pMem,
		strlen(pMem),
		NULL,
		NULL,
		NULL,
		"fx_4_0",
		dwShaderFlags,
		0,
		m_pD3D10Device,
		NULL,
		NULL,
		&m_pTempEffect,
		&pErrorBuffer,
		NULL)))
	{
		OutputDebugStringA((char*)pErrorBuffer->GetBufferPointer());
		return false;
	}

	m_pTempTechnique = m_pTempEffect->GetTechniqueByName("Render");
	return true;
}	

bool D3D10Renderer::createBuffer(){
	
	Vertex verts[] = {
		{-1.0f,-1.0f,0.0f},
		{0.0f,1.0f,0.0f},
		{1.0f,-1.0f,0.0f}
	};

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex)*3;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &verts;

	if (FAILED(m_pD3D10Device->CreateBuffer(&bd,&InitData,&m_pTempBuffer)))
	{
		OutputDebugStringA("Can't create buffer");
	}

return true;
}
bool D3D10Renderer::createVertexLayout(){
	UINT numElements = sizeof(VertexLayout)/ sizeof(D3D10_INPUT_ELEMENT_DESC);
	D3D10_PASS_DESC PassDesc;
	m_pTempTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);

	if (FAILED(m_pD3D10Device->CreateInputLayout(
		VertexLayout,
		numElements,
		PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize,
		&m_pTempVertexLayout)))
	{
		OutputDebugStringA("Can't create layout");
	}
return true;
}