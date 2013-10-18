#include "D3D10Renderer.h"

//Include header files from DX10 library


struct Vertex {
	float x, y, z, tu, tv;
};

const D3D10_INPUT_ELEMENT_DESC VertexLayout[] =  //description of an array of elements for the input-assembler stage.
{
	{"POSITION",							//SemanticName
	0,										//SemanticIndex
	DXGI_FORMAT_R32G32B32_FLOAT,			//data type of the element data.
	0,										//an integer value that identifies the input-assembler 
	0,										//Offset (in bytes) between each element.
	D3D10_INPUT_PER_VERTEX_DATA,			//input data class for a single input slot
	0},	//number of instances to draw before stepping one unit forward in a vertex buffer
	{"TEXCOORD",
	0,
	DXGI_FORMAT_R32G32_FLOAT,
	0,
	12,
	D3D10_INPUT_PER_VERTEX_DATA,
	0}
};

const char basicEffect[]=\
	"float4 VS( float4 Pos : POSITION ) : SV_POSITION"\
	"{"\
	"	return Pos;"\
	"}"\
	"float4 PS( float4 Pos : SV_POSITION ) : SV_Target"\
	"{"\
	"	return float4( 1.0f, 1.0f, 0.0f, 1.0f );"\
	"}"\
	"technique10 Render"\
	"{"\
	"	pass P0"\
	"	{"\
	"		SetVertexShader( CompileShader( vs_4_0, VS() ) );"\
	"		SetGeometryShader( NULL );"\
	"		SetPixelShader( CompileShader( ps_4_0, PS() ) );"\
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
	m_pTempBuffer = NULL;
	m_pTempVertexLayout = NULL;

	m_View = XMMatrixIdentity();
	m_Projection = XMMatrixIdentity();
	m_World  = XMMatrixIdentity();
}

//Destructor
D3D10Renderer::~D3D10Renderer()
{

	
	if (m_pD3D10Device)
		m_pD3D10Device->ClearState();

	//Release each of the DX10 interfaces
	if(m_pTempEffect)
		m_pTempEffect->Release(); //Release the pointer when no referenced objects remain
		if(m_pTempVertexLayout)
		m_pTempVertexLayout->Release();
	if(m_pTempBuffer)
		m_pTempBuffer->Release();
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

	XMFLOAT3 cameraPos = XMFLOAT3(0.0f, 0.0f, -10.0f);
	XMFLOAT3 focusPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	createCamera(XMLoadFloat3(&cameraPos), XMLoadFloat3(&focusPos), XMLoadFloat3(&up), XM_PI/4, (float)width/(float)height, 0.1f, 100.0f);
	positionObject(0.0f, 0.0f, 0.0f);

	//Calls functions in D3D10Renderer.
	if(!createDevice(window, width, height, fullScreen))
		return false;
	if(!createInitialRenderTarget(width, height))
		return false;

	//if(!loadEffectFromMemory(basicEffect))
	if(!loadEffectFromFile("../Debug/Effects/Texture.fx"))
		return false;
	 if (!loadBaseTexture("../Debug/Textures/face.png"))
                return false;
	if(!createVertexLayout())
		return false;
	if(!createBuffer())
		return false;
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

	
	DXGI_SWAP_CHAIN_DESC sd;		//A struct which describes a swap chain
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		//Describes the surface usage and CPU access options for the back buffer
	
	if(fullScreen)
		sd.BufferCount = 2;		// Number of buffers in swap chain.
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



	if(FAILED(D3D10CreateDeviceAndSwapChain(	//Create a Direct3D 10.0 device and a swap chain.
		NULL, 
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
	ID3D10Texture2D *pBackBuffer;		//Interface for a 2d texture used as a buffer

	
	if (FAILED(m_pSwapChain->GetBuffer(		//Function to access one of the swapchains back buffers
		0,									//IN- The buffer index
		__uuidof(ID3D10Texture2D),			//IN -  Type of interface used to manipulate the buffer
		(void**)&pBackBuffer)))				//IN-OUT - pointer to back buffer interface
		return false;



	D3D10_TEXTURE2D_DESC descDepth;		//Struct which describes a 2d texture
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



	if (FAILED(m_pD3D10Device->CreateTexture2D(		//Method to create an array of 2d textures
		&descDepth,									//IN - pointer to 2d texture description
		NULL,										//IN - pointer to array of subresource descriptions
		&m_pDepthStencilTexture)))					//OUT - address of pointer to created texture
		return false;

	//creates the DepthStencilView and the RenderTargetView, binds them both to the
	//pipeline and then finally creates a viewport; which is used in the pipeline for some of the transformations
	

	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;		//struct which specifies the subresource(s) from a texture that are accessible using a depth-stencil view.
	descDSV.Format=descDepth.Format;
	descDSV.ViewDimension=D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice=0;


	if (FAILED
		(m_pD3D10Device->CreateDepthStencilView(	//Method which creates a depth-stencil view for accessing resource data.
			m_pDepthStencilTexture,
			&descDSV,&m_pDepthStencelView)))
		return false;

	if (FAILED(m_pD3D10Device->CreateRenderTargetView( //Create a render-target view for accessing resource data.
		pBackBuffer, //in - vPointer to the resource that will serve as the render target. 
		NULL, //in-render-target-view description
		&m_pRenderTargetView )))//out - renderTargetView
	{
            pBackBuffer->Release();
			return  false;
	}
       pBackBuffer->Release();
	   

	m_pD3D10Device->OMSetRenderTargets(			//Method to bind one or more render targets and the depth-stencil buffer to the output-merger stage.
		1,										//Number of render targets to bind.
		&m_pRenderTargetView,					//Pointer to an array of render targets.
		m_pDepthStencelView						//Pointer to a depth-stencil view.
		);


	D3D10_VIEWPORT vp;							//Struct which defines the dimensions of a viewport
   	vp.Width = windowWidth;
    vp.Height = windowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    

	m_pD3D10Device->RSSetViewports(				//Method which binds an array of viewports to the rasterizer stage of the pipeline.
		1, 
		&vp 
		);
	return true;
}

bool D3D10Renderer::loadEffectFromMemory(const char* pMem){

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(DEBUG)||defined(_DEBUG)
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif
	
	ID3D10Blob * pErrorBuffer = NULL;

	if (FAILED(D3DX10CreateEffectFromMemory(  //Create an effect from memory.
		pMem,					//IN - effect in memory.
		strlen(pMem),			//IN - Size of the effect
		NULL,					//IN - Name of the effect file
		NULL,					//IN - A NULL-terminated array of shader macros
		NULL,					//IN - an include interface
		"fx_4_0",				//IN - shader profile, or shader model.
		dwShaderFlags,			//IN - HLSL compile options
		0,						//IN - Effect compile options
		m_pD3D10Device,			//IN - the device that will use the resources.
		NULL,					//IN - an effect pool for sharing variables between effects.
		NULL,					//IN - a thread pump interface. Use NULL to specify that this function should not return until it is completed.
		&m_pTempEffect,			//OUT -  the effect that is created.
		&pErrorBuffer,			//OUT -  memory (see ID3D10Blob Interface) that contains effect compile errors
		NULL)))					//OUT - the return value.
	{
		OutputDebugStringA((char*)pErrorBuffer->GetBufferPointer());
		return false;
	}

	m_pTempTechnique = m_pTempEffect->GetTechniqueByName("Render");
	return true;
}	


bool D3D10Renderer::loadEffectFromFile(char* pFilename)
{
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(DEBUG)||defined(_DEBUG)
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif
	
	ID3D10Blob * pErrorBuffer = NULL;

	if(FAILED(D3DX10CreateEffectFromFileA(
		pFilename, 
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

	m_pWorldEffectVariable = m_pTempEffect->GetVariableByName("matWorld")->AsMatrix();
	m_pViewEffectVariable = m_pTempEffect->GetVariableByName("matView")->AsMatrix();
	m_pProjectionEffectVariable = m_pTempEffect->GetVariableByName("matProjection")->AsMatrix();

	m_pTempTechnique = m_pTempEffect->GetTechniqueByName("Render");
	m_pBaseTextureEffectVariable = m_pTempEffect->GetVariableByName("diffuseMap")->AsShaderResource();
	return true;
}



bool D3D10Renderer::createBuffer(){
	
	Vertex verts[] = {
		{-1.0f,-1.0f,0.0f,0.0f, 1.0f},	//Bottom left point
		{-1.0f,1.0f,0.0f, 0.0f, 0.0f},	//Top point
		{1.0f,-1.0f,0.0f, 1.0f, 1.0f},
		{1.0f,1.0f,0.0f, 1.0f, 0.0f}//Bottom right point
	};

	

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;					//how the buffer is expected to be read from and written to.
	bd.ByteWidth = sizeof(Vertex)*4;				//Size of the buffer in bytes.
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;		//how the buffer will be bound to the pipeline.
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

	m_pWorldEffectVariable->SetMatrix((float*)&m_World);
	m_pViewEffectVariable->SetMatrix((float*)&m_View);
	m_pProjectionEffectVariable->SetMatrix((float*)&m_Projection);

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

void D3D10Renderer::render()
{
	//Triangle
	//m_pD3D10Device->IASetPrimitiveTopology(		//Bind information about the primitive type, and data order that describes input data for the input assembler stage.
	//D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);    
	//m_pD3D10Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);			//Point
	//m_pD3D10Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);			//Line
	
	m_pBaseTextureEffectVariable->SetResource(m_pBaseTextureMap);
	m_pD3D10Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pD3D10Device->IASetInputLayout(	//Bind an input-layout object to the input-assembler stage.
		m_pTempVertexLayout);			//IN - Input layout object

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	m_pD3D10Device->IASetVertexBuffers(		//Bind an array of vertex buffers to the input-assembler stage.
		0,									//IN - The first input slot for binding
		1,									//IN - The number of vertex buffers in the array.
		&m_pTempBuffer,						//IN - An array of vertex buffers
		&stride,							//IN - an array of stride values; one stride value for each buffer in the vertex-buffer array. 
													//Each stride is the size (in bytes) of the elements that are to be used from that vertex buffer.
		&offset);							//IN - array of offset values; one offset value for each buffer in the vertex-buffer array. 
													//Each offset is the number of bytes between the first element of a vertex buffer and the first element that will be used.

	D3D10_TECHNIQUE_DESC techniqueDesc;
	m_pTempTechnique->GetDesc(&techniqueDesc);

	for(unsigned int i=0;i<techniqueDesc.Passes;++i)
	{
		ID3D10EffectPass *pCurrentPass= m_pTempTechnique->GetPassByIndex(i);
		
		pCurrentPass->Apply(0);		//Set the state contained in a pass to the device.
			
		m_pD3D10Device->Draw(		//Draw non-indexed, non-instanced primitives.
			4,						//Number of vertices to draw.
			0);						//Index of the first vertex
	}

}

void D3D10Renderer::present()
{

	//Present================
	//Presents a rendered image to the user.

	//Swaps the buffers in the chain, the back buffer to the front(screen)
	//http://msdn.microsoft.com/en-us/library/bb174576%28v=vs.85%29.aspx - BMD
    m_pSwapChain->Present( 0, 0 );
}

void D3D10Renderer::createCamera(XMVECTOR &position, XMVECTOR &focus, XMVECTOR &up, float fov, float aspectRatio, float nearClip, float farClip){
	
	m_View = XMMatrixLookAtLH(position, focus, up);

	m_Projection = XMMatrixPerspectiveFovLH(fov, aspectRatio,nearClip,farClip);
}
void D3D10Renderer::positionObject(float x, float y, float z)
{
	m_World = XMMatrixTranslation(x,y,z);
}

bool D3D10Renderer::loadBaseTexture(char* pFilename)
{
	if(FAILED(D3DX10CreateShaderResourceViewFromFileA(m_pD3D10Device,
		pFilename,
		NULL,
		NULL,
		&m_pBaseTextureMap,
		NULL)))
{
	return false;
	}
	return true;
}





