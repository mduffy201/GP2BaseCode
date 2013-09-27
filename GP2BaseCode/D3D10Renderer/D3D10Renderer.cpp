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