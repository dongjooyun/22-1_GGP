#pragma once

#include "Common.h"
#include "Window/MainWindow.h"

namespace library
{

	class Renderer
	{
	public:
		Renderer();
		~Renderer();
		HRESULT Initialize(_In_ HWND hWnd);
		void Render();
	protected:
		D3D_DRIVER_TYPE m_driverType;
		D3D_FEATURE_LEVEL m_featureLevel;
		ComPtr<ID3D11Device> m_d3dDevice;
		ComPtr<ID3D11Device1> m_d3dDevice1;
		ComPtr<ID3D11DeviceContext> m_immediateContext;
		ComPtr<ID3D11DeviceContext1> m_immediateContext1;
		ComPtr<IDXGISwapChain> m_swapChain;
		ComPtr<IDXGISwapChain1> m_swapChain1;
		ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	};
}
